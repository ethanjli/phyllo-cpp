#pragma once

// Standard libraries

// Third-party libraries

// Phyllo
#include "Phyllo/Types.h"
#include "Phyllo/Protocol/Types.h"

// Frame layer handles data framing

namespace Phyllo { namespace Protocol { namespace Transport {

template<typename Stream>
class StreamLink {
  public:
    Stream *stream;

    using ToReceive = void; // This link does not receive data from a lower link, it is at the bottom
    using Receive = uint8_t;
    using OptionalReceive = void; // This link does not expose an optional-receive interface
    using Send = ByteBufferView;
    using SendDelegate = etl::delegate<bool(const Send &, DataUnitTypeCode)>;
    using ToSend = void; // This link does not send data to a lower link, it is at the bottom
    using ToSendDelegate = void;

    static const long kTimeout = 0;

    StreamLink(Stream &stream) : stream(&stream) {}
    StreamLink(Stream *stream) : stream(stream) {}

    // Event loop interface

    void setup() {
      setTimeout();
    };
    void update() {}

    // Peek interface

    int hasRead();

    Receive peek() const;

    void consume();

    // Read interface

    Receive read();
    size_t read(ByteBuffer &buffer, size_t maxLength) {
      if (buffer.max_size() < maxLength) maxLength = buffer.max_size();
      buffer.resize(maxLength);
      size_t bytesRead = read(buffer.data(), maxLength);
      buffer.resize(bytesRead);
      return bytesRead;
    }

    // ByteBufferLink interface

    bool send(const ByteBufferView &buffer, uint8_t type = DataUnitType::Bytes::Stream);
    bool send(const uint8_t *buffer, size_t size, uint8_t type = DataUnitType::Bytes::Stream);
    bool send(uint8_t byte, uint8_t type = DataUnitType::Bytes::Stream);

  protected:
    size_t read(Receive *buffer, size_t maxLength);
    void setTimeout();
};

// Buffered stream reading for more efficient serial reading over USB connections

template<size_t BufferSize = 64>
class BufferedStreamLink {
  public:
    using ToReceive = FixedByteBuffer<BufferSize>;
    using Receive = ByteBufferView;
    using OptionalReceive = Util::Optional<Receive>;
    using Send = ByteBufferView;
    using SendDelegate = etl::delegate<bool(const Send &, DataUnitTypeCode)>;
    using ToSend = ByteBufferView;
    using ToSendDelegate = etl::delegate<bool(const ToSend &, DataUnitTypeCode)>;

    BufferedStreamLink(const ToSendDelegate &delegate) : sender(delegate) {}
    BufferedStreamLink(const BufferedStreamLink &chunkLink) = delete; // prevent accidental copy-by-value

    size_t toReceiveMaxLength() const {
      return readBuffer.max_size() - readBuffer.size();
    }

    bool full() const {
      return readBuffer.full();
    }

    // Event loop interface

    void setup() {}
    void update() {}

    // Peek interface

    size_t hasRead() const {
      if (cursor >= readBuffer.size()) return 0;
      return readBuffer.size() - cursor;
    }

    uint8_t peek() const { // Note: this is only valid when hasRead is true - check that first!
      return readBuffer[cursor];
    }

    void consume() {
      ++cursor;
      if (cursor < readBuffer.size()) return;

      cursor = 0;
      readBuffer.clear();
    }

    // ReadInterface

    uint8_t read() {
      uint8_t readByte = peek();
      consume();
      return readByte;
    }
    ByteBufferView read(size_t bytesConsumed) {
      bytesConsumed = min(bytesConsumed, readBuffer.size() - cursor);
      ByteBufferView buffer(readBuffer.begin() + cursor, bytesConsumed);
      cursor += bytesConsumed;
      return buffer;
    }

    // ByteBufferLink interface

    bool receive(uint8_t streamByte) {
      if (readBuffer.full()) return false;
      readBuffer.push_back(streamByte);
      return true;
    }
    Receive receive(const ByteBufferView &received) {
      // Any bytes which don't fit in the buffer will be silently discarded!
      // Use toReceiveMaxLength to ensure that we don't try to receive any bytes
      // which won't fit in the buffer.
      auto appendLocation = readBuffer.end();
      size_t readLength = received.size();
      if (toReceiveMaxLength() < readLength) readLength = toReceiveMaxLength();
      readBuffer.resize(readBuffer.size() + readLength);
      memcpy(appendLocation, received.data(), readLength);
      return ByteBufferView(readBuffer.begin() + cursor, readBuffer.end());
    }

    // ByteBufferLink interface

    bool send(const ByteBufferView &payload, DataUnitTypeCode type = DataUnitType::Bytes::Chunk) {
      return sender(payload, DataUnitType::Bytes::Stream);
    }

  protected:
    const ToSendDelegate &sender;

    FixedByteBuffer<BufferSize> readBuffer;
    size_t cursor = 0;
};

} } }