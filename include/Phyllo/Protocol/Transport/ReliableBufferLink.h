#pragma once

// Standard libraries

// Third-party libraries
#include <etl/delegate.h>

// Phyllo
#include "Phyllo/Types.h"
#include "Phyllo/Util/Optional.h"
#include "DatagramLink.h"
#include "ReliableBuffer.h"
#include "ARQ.h"

// ReliableBuffer layer handles reliableBuffer resending
// WARNING: implementation is incomplete!

namespace Phyllo { namespace Protocol { namespace Transport {

class ReliableBufferLink {
  public:
    using ToReceive = ByteBufferView; // The type of data passed up from below
    using Receive = ReliableBuffer; // The type of data passed up to above
    using OptionalReceive = Util::Optional<Receive>;
    using Send = ByteBufferView; // The type of data passed down from above
    using SendDelegate = etl::delegate<bool(const Send &, DataUnitTypeCode, bool)>;
    using ToSend = ByteBufferView; // The type of data passed down to below
    using ToSendDelegate = etl::delegate<bool(const ToSend &, DataUnitTypeCode)>;

    ReliableBufferLink(const ToSendDelegate &delegate) : 
      arqReceiver(delegate), sender(delegate) {}

    ReliableBufferLink(const ReliableBufferLink &reliableBufferLink) = delete; // prevent accidental copy-by-value

    // Event loop interface

    void setup() {
      arqSender.setup();
      arqReceiver.setup();
    }

    void update() {
      arqReceiver.update();
      arqSender.update(); // TODO: send the next datagramToSend in arqSender if it's readyToSend
    }

    // ByteBufferLink interface 

    OptionalReceive receive(const ByteBufferView &buffer, DataUnitTypeCode type) {
      OptionalReceive received;
      if (
        type != DataUnitType::Transport::ReliableBuffer
        || buffer.empty()
        || !received->read(buffer)
      ) return received;

      arqSender.receive(*received);
      received.enabled = arqReceiver.receive(*received);
      arqReceiver.update();
      return received;
    }

    bool send(
      const ByteBufferView &payload, DataUnitTypeCode type = DataUnitType::Bytes::Buffer, bool reliable = true
    ) { // TODO: implement unreliable transmission option
      if (payload.empty()) return false;
      if (payload.size() > ReliableBuffer::kPayloadSizeLimit) return false;

      ReliableBuffer reliableBuffer;
      reliableBuffer.header.seqNum = counter; // TODO: outsource this to GBNSender as updateSend
      arqReceiver.prepare(reliableBuffer.header); // update the acknowledgement-related fields
      if (!reliableBuffer.write(payload)) return false;
      reliableBuffer.write(payload);

      if (!sender(reliableBuffer.buffer(), ReliableBuffer::kType)) return false; // TODO: actually we just want to queue into arqSender
      
      counter = (counter + 1) % 256; // TODO: outsource this to GBNSender with enqueue
      arqReceiver.sent(reliableBuffer.header);
      return true;
    }

    // ReliableBufferLink interface

    bool reliableReceived() const {
      return true; // TODO: implement by checking the header's seq flag
    }

  protected:
    GBNSender arqSender;
    GBNReceiver arqReceiver;

    uint8_t counter = 0;

    const ToSendDelegate &sender;
};

} } }