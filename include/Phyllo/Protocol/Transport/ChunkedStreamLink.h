#pragma once

// Standard libraries

// Third-party libraries
#include <etl/algorithm.h>
#include <etl/array.h>
#include <etl/delegate.h>
#include <PacketSerial.h>

// Phyllo
#include "Phyllo/Types.h"
#include "Phyllo/Util/Optional.h"
#include "Phyllo/Protocol/Types.h"
#include "StreamLink.h"

// Chunked Stream layer handles data framing

#ifndef PHYLLO_TRANSPORT_CHUNK_SIZE_LIMIT
#define PHYLLO_TRANSPORT_CHUNK_SIZE_LIMIT 255  // For USB, 255 ensures that the starting delimiter won't be translated as its own USB packet, for higher performance on max-length chunks
#endif

namespace Phyllo { namespace Protocol { namespace Transport {

class ChunkedStreamLink {
  public:
    static const uint8_t kChunkMarker = '\0'; // string null terminator
    static const size_t kSizeLimit = PHYLLO_TRANSPORT_CHUNK_SIZE_LIMIT; // chosen for best efficiency with COBS encoding
    static_assert(kSizeLimit <= 256, "Chunk size limit cannot exceed 256!");
    static const size_t kOverheadSize = 1; // chunk end delimiter overhead for payload; do not change!
    static const size_t kPayloadSizeLimit = kSizeLimit - kOverheadSize;

    using ToReceive = uint8_t;
    /*using Receive = FixedByteBuffer<kPayloadSizeLimit>;*/
    using Receive = ByteBufferView;
    using OptionalReceive = Util::Optional<Receive>;
    using Send = ByteBufferView;
    using SendDelegate = etl::delegate<bool(const Send &, DataUnitTypeCode)>;
    using ToSend = ByteBufferView;
    using ToSendDelegate = etl::delegate<bool(const ToSend &, DataUnitTypeCode)>;

    ChunkedStreamLink(const ToSendDelegate &delegate) : 
      sender(delegate),
      kChunkMarkerBufferView(kChunkMarkerBuffer.begin(), kChunkMarkerBuffer.end()) {}
    ChunkedStreamLink(const ChunkedStreamLink &chunkLink) = delete; // prevent accidental copy-by-value

    // Peek interface

    ByteBufferView peek() const {
      // This doesn't check if we have a complete chunk yet - call hasRead() first to check that!
      return ByteBufferView(receivedBuffer);
    }

    bool hasRead() const {
      return receivedChunk;
    }

    void consume() {
      receivedChunk = false;
      receivedBufferOverflowed = false;
      receivedBuffer.clear();
    }

    // Event loop interface

    void setup() {}
    void update() {}
    
    // ByteBufferLink interface

    OptionalReceive receive(ToReceive streamByte) {
      // Note: after the output is used, the consume method must be called to reset state so that the next byte can be taken
      OptionalReceive received;
      if (streamByte == kChunkMarker) {
        receiveChunkMarker();
        received.enabled = receivedChunk;
        received.value = peek();
      } else {
        receiveGenericByte(streamByte);
      }
      return OptionalReceive();
    }

    bool overflowReceived() const {
      return receivedBufferOverflowed;
    }

    bool send(const ByteBufferView &payload, DataUnitTypeCode type = DataUnitType::Transport::Frame) {
      if (payload.empty()) return false;
      if (payload.size() > kPayloadSizeLimit) return false;

      sendChunkMarker();
      bool sendStatus = sender(payload, DataUnitType::Bytes::Stream);
      sendChunkMarker();
      return sendStatus;
    }

  protected:
    using FixedChunk = FixedByteBuffer<kSizeLimit>;

    const ToSendDelegate &sender;

    const etl::array<uint8_t, 1> kChunkMarkerBuffer = {{ kChunkMarker }};
    const ByteBufferView kChunkMarkerBufferView;

    FixedChunk receivedBuffer;
    bool receivedBufferOverflowed = false;
    bool receivedChunk = false;

    void receiveChunkMarker() {
      receivedChunk = !receivedBuffer.empty();
    }

    void receiveGenericByte(uint8_t receivedByte) {
      receivedBufferOverflowed = receivedBuffer.full();
      if (!receivedBufferOverflowed) receivedBuffer.push_back(receivedByte);
    }

    void sendChunkMarker() {
      sender(kChunkMarkerBufferView, DataUnitType::Bytes::Stream);
    }
};

} } }