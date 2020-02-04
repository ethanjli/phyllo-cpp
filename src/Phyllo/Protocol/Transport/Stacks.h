#pragma once

// Standard libraries

// Third-party libraries
#include <etl/delegate.h>

// Phyllo
#include "Phyllo/Types.h"
#include "Phyllo/Protocol/Transport/ReliableBufferLink.h"

// Stacks orchestrate the flow of data through protocol layers

namespace Phyllo { namespace Protocol { namespace Transport {

// Medium stacks consist of the lower transport layers which make it possible to send byte buffers

template<typename Stream>
class StreamMediumStack {
  public:
    using TopLink = FrameLink;
    using BottomLink = StreamLink<Stream>;

    using ToReceive = typename BottomLink::ToReceive;
    using Receive = TopLink::Receive; // The type of data passed up to above
    using OptionalReceive = TopLink::OptionalReceive;
    using Send = TopLink::Send; // The type of data passed down from above
    using SendDelegate = TopLink::SendDelegate;
    using ToSend = typename BottomLink::ToSend;
    using ToSendDelegate = void;

    static const size_t kStreamBufferSize = ChunkedStreamLink::kSizeLimit + 1;

    StreamLink<Stream> stream;
    BufferedStreamLink<kStreamBufferSize> buffered;
    ChunkedStreamLink chunk;
    FrameLink frame;

    TopLink &top;
    BottomLink &bottom;
    SendDelegate sender;

    StreamMediumStack(Stream *stream) : StreamMediumStack(*stream) {}
    StreamMediumStack(Stream &stream) :
      stream(stream), buffered(intermediateToSender),
      chunk(intermediateToSender), frame(intermediateToSender),
      top(frame), bottom(this->stream),
      sender(SendDelegate::create<TopLink, &TopLink::send>(top)) {
        intermediateToSender = IntermediateToSendDelegate::create<
          StreamMediumStack, &StreamMediumStack::toSend
        >(*this);
      }

    // Event loop interface

    void setup() {
      stream.setup();
      buffered.setup();
      chunk.setup();
      frame.setup();
    }
    void update() {
      stream.update();
      buffered.update();
      chunk.update();
      frame.update();
    }

    // ByteBufferLink interface

    OptionalReceive receive() {
      while (true) {
        while (buffered.hasRead() && !chunk.hasRead()) chunk.receive(buffered.read());
        fillStreamBuffer();
        if (chunk.hasRead() || buffered.full() || !stream.hasRead()) break; // nothing left to do this cycle
      }
      if (!chunk.hasRead()) return OptionalReceive();

      auto frameReceived = frame.receive(chunk.peek());
      chunk.consume(); // clear the chunk so we're ready to receive another one
      return frameReceived;
    }

    bool send(const ByteBufferView &payload, DataUnitTypeCode type = DataUnitType::Bytes::Buffer) {
      return top.send(payload, type);
    }

  protected:
    using IntermediateToSendDelegate = etl::delegate<bool(const ByteBufferView &, DataUnitTypeCode)>;
    IntermediateToSendDelegate intermediateToSender;

    bool toSend(const ByteBufferView &buffer, DataUnitTypeCode type) {
      switch (type) {
        case DataUnitType::Bytes::Stream:
        case DataUnitType::Bytes::Chunk:
          return stream.send(buffer, type);
        case DataUnitType::Transport::Frame:
          return chunk.send(buffer, type);
        default:
          return top.send(buffer, type);
      }
    }

    size_t fillStreamBuffer() {
      while (stream.hasRead() && !buffered.full()) {
        BufferedStreamLink<kStreamBufferSize>::ToReceive toReceive;
        stream.read(toReceive, buffered.toReceiveMaxLength());
        buffered.receive(ByteBufferView(toReceive));
      }
      return buffered.hasRead();
    }
};

// Logical stacks consist of the upper transport layers which provide various transport-level services

class MinimalLogicalStack {
  public:
    using TopLink = DatagramLink;
    using BottomLink = DatagramLink;

    using ToReceive = BottomLink::ToReceive; // The type of data passed up from below
    using Receive = TopLink::Receive; // The type of data passed up to above
    using OptionalReceive = TopLink::OptionalReceive;
    using Send = TopLink::Send; // The type of data passed down from above
    using SendDelegate = TopLink::SendDelegate;
    using ToSend = BottomLink::ToSend; // The type of data passed down to below
    using ToSendDelegate = BottomLink::ToSendDelegate;

    DatagramLink datagram;

    TopLink &top;
    BottomLink &bottom;
    SendDelegate sender;

    MinimalLogicalStack(const ToSendDelegate &toSender) :
      datagram(toSender),
      top(datagram), bottom(datagram),
      sender(SendDelegate::create<TopLink, &TopLink::send>(top)) {}

    void setup() {
      datagram.setup();
    }
    void update() {
      datagram.update();
    }

    // Event loop interface

    OptionalReceive receive(const ByteBufferView &buffer) {
      return datagram.receive(buffer);
    }

    // ByteBufferLink interface

    bool send(const ByteBufferView &payload, DataUnitTypeCode type = DataUnitType::Bytes::Buffer) {
      return top.send(payload, type);
    }
};

class ReducedLogicalStack {
  public:
    using TopLink = ValidatedDatagramLink;
    using BottomLink = MinimalLogicalStack::BottomLink;

    using ToReceive = BottomLink::ToReceive; // The type of data passed up from below
    using Receive = TopLink::Receive; // The type of data passed up to above
    using OptionalReceive = TopLink::OptionalReceive;
    using Send = TopLink::Send; // The type of data passed down from above
    using SendDelegate = TopLink::SendDelegate;
    using ToSend = BottomLink::ToSend; // The type of data passed down to below
    using ToSendDelegate = BottomLink::ToSendDelegate;

    MinimalLogicalStack minimal;
    ValidatedDatagramLink validated;

    TopLink &top;
    BottomLink &bottom;
    SendDelegate sender;

    ReducedLogicalStack(const ToSendDelegate &toSender) :
      minimal(toSender), validated(minimal.sender),
      top(validated), bottom(minimal.bottom),
      sender(SendDelegate::create<TopLink, &TopLink::send>(top)) {}

    void setup() {
      minimal.setup();
      validated.setup();
    }
    void update() {
      minimal.update();
      validated.update();
    }

    // Event loop interface

    OptionalReceive receive(const ByteBufferView &buffer) {
      auto minimalReceived = minimal.receive(buffer);
      if (!minimalReceived) return OptionalReceive();

      return validated.receive(
        getPayload(*minimalReceived), getPayloadType(*minimalReceived)
      );
    }

    // ByteBufferLink interface

    bool send(const ByteBufferView &payload, DataUnitTypeCode type = DataUnitType::Bytes::Buffer) {
      return top.send(payload, type);
    }
};

class StandardLogicalStack {
  public:
    using TopLink = ReliableBufferLink;
    using BottomLink = ReducedLogicalStack::BottomLink;

    using ToReceive = BottomLink::ToReceive; // The type of data passed up from below
    using Receive = TopLink::Receive; // The type of data passed up to above
    using OptionalReceive = TopLink::OptionalReceive;
    using Send = TopLink::Send; // The type of data passed down from above
    using SendDelegate = TopLink::SendDelegate;
    using ToSend = BottomLink::ToSend; // The type of data passed down to below
    using ToSendDelegate = BottomLink::ToSendDelegate;

    ReducedLogicalStack reduced;
    ReliableBufferLink reliable;

    TopLink &top;
    BottomLink &bottom;
    SendDelegate sender;

    StandardLogicalStack(const ToSendDelegate &toSender) :
      reduced(toSender), reliable(reduced.sender),
      top(reliable), bottom(reduced.bottom),
      sender(SendDelegate::create<TopLink, &TopLink::send>(top)) {}

    void setup() {
      reduced.setup();
      reliable.setup();
    }

    void update() {
      reduced.update();
      reliable.update();
    }

    // Event loop interface

    OptionalReceive receive(const ByteBufferView &buffer) {
      reliable.update();
      auto reducedReceived = reduced.receive(buffer);
      if (!reducedReceived) return OptionalReceive();

      return reliable.receive(
        getPayload(*reducedReceived),
        getPayloadType(*reducedReceived)
      );
    }

    // ByteBufferLink interface

    bool send(const ByteBufferView &payload, DataUnitTypeCode type = DataUnitType::Bytes::Buffer) {
      return top.send(payload, type);
    }
};

template<typename MediumStack, typename LogicalStack>
class TransportStack {
  public:
    using TopLink = typename LogicalStack::TopLink;
    using BottomLink = typename MediumStack::BottomLink;

    using ToReceive = typename BottomLink::ToReceive; // The type of data passed up from below
    using Receive = typename TopLink::Receive; // The type of data passed up to above
    using OptionalReceive = typename TopLink::OptionalReceive;
    using Send = typename TopLink::Send; // The type of data passed down from above
    using SendDelegate = typename TopLink::SendDelegate;
    using ToSend = typename BottomLink::ToSend; // The type of data passed down to below
    using ToSendDelegate = typename BottomLink::ToSendDelegate;

    MediumStack &medium;
    LogicalStack &logical;

    TopLink &top;
    BottomLink &bottom;
    SendDelegate &sender;

    TransportStack(MediumStack &medium, LogicalStack &logical) :
      medium(medium), logical(logical),
      top(logical.top), bottom(medium.bottom),
      sender(logical.sender) {}

    void setup() {
      medium.setup();
      logical.setup();
    }

    void update() {
      medium.update();
      logical.update();
    }

    // Event loop interface

    OptionalReceive receive() {
      auto mediumReceived = medium.receive();
      if (!mediumReceived) return OptionalReceive();

      return logical.receive(getPayload(*mediumReceived));
    }

    // ByteBufferLink interface

    bool send(const ByteBufferView &payload, DataUnitTypeCode type = DataUnitType::Bytes::Buffer) {
      return top.send(payload, type);
    }
};

} } }