#pragma once

// Standard libraries

// Third-party libraries
#include <PacketSerial.h>
#include <etl/delegate.h>

// Phyllo
#include "Phyllo/Types.h"
#include "Phyllo/Util/Optional.h"
#include "Phyllo/Protocol/Types.h"
#include "ChunkedStreamLink.h"

// Frame layer handles data framing

namespace Phyllo { namespace Protocol { namespace Transport {

class FrameLink {
  public:
    using Encoder = COBS;

    static const size_t kOverheadSize = 1; // max COBS encoding overhead for payload; do not change!
    static const size_t kPayloadSizeLimit = ChunkedStreamLink::kPayloadSizeLimit - kOverheadSize;

    using ToReceive = ByteBufferView;
    using Receive = FixedByteBuffer<kPayloadSizeLimit>;
    using OptionalReceive = Util::Optional<Receive>;
    using Send = ByteBufferView;
    using SendDelegate = etl::delegate<bool(const Send &, DataUnitTypeCode)>;
    using ToSend = ByteBufferView;
    using ToSendDelegate = etl::delegate<bool(const ToSend &, DataUnitTypeCode)>;

    FrameLink(const ToSendDelegate &delegate) : sender(delegate) {}
    FrameLink(const FrameLink &link) = delete; // prevent accidental copy-by-value

    // Event loop interface

    void setup() {}
    void update() {}

    // ByteBufferLink interface

    OptionalReceive receive(const ByteBufferView &buffer) {
      OptionalReceive received;
      if (buffer.empty()) return received;

      // Allow access of payload
      received->resize(kPayloadSizeLimit); // make receivedPayload look like an array for Encoder::decode
      size_t payloadReceivedSize = Encoder::decode( // decode buffer contents as frame payload
        buffer.data(), buffer.size(), received->data()
      );
      received->resize(payloadReceivedSize); // make receivedPayload look like a vector for consumers
      
      received.enabled = payloadReceivedSize > 0;
      return received;
    }

    bool send(const ByteBufferView &payload, DataUnitTypeCode type = DataUnitType::Transport::Datagram) {
      if (payload.empty()) return false;
      if (payload.size() > kPayloadSizeLimit) return false;

      FixedFrame encodeBuffer;
      encodeBuffer.resize(ChunkedStreamLink::kPayloadSizeLimit);
      size_t encodedSize = Encoder::encode(payload.data(), payload.size(), encodeBuffer.data());
      encodeBuffer.resize(encodedSize);

      bool sendStatus = sender(ByteBufferView(encodeBuffer), DataUnitType::Transport::Frame);
      return sendStatus;
    }

  protected:
    using FixedFrame = FixedByteBuffer<ChunkedStreamLink::kPayloadSizeLimit>;
    const ToSendDelegate &sender;
};

} } }