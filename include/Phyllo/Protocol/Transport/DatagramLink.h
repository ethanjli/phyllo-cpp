#pragma once

// Standard libraries

// Third-party libraries
#include <etl/delegate.h>

// Phyllo
#include "Phyllo/Types.h"
#include "Phyllo/Util/Optional.h"
#include "Phyllo/Protocol/Types.h"
#include "Datagram.h"
#include "FrameLink.h"

// Datagram layer handles wrapping of data with header fields to enable validation of length and contents

namespace Phyllo { namespace Protocol { namespace Transport {

class DatagramLink {
  public:
    using ToReceive = ByteBufferView; // The type of data passed up from below
    using Receive = Datagram; // The type of data passed up to above
    using OptionalReceive = Util::Optional<Receive>;
    using Send = ByteBufferView; // The type of data passed down from above
    using SendDelegate = etl::delegate<bool(const Send &, DataUnitTypeCode)>;
    using ToSend = ByteBufferView; // The type of data passed down to below
    using ToSendDelegate = etl::delegate<bool(const ToSend &, DataUnitTypeCode)>;

    DatagramLink(const ToSendDelegate &delegate) : sender(delegate) {}
    DatagramLink(const DatagramLink &datagramLink) = delete; // prevent accidental copy-by-value

    // Event loop interface

    void setup() {}
    void update() {}

    // ByteBufferLink interface

    OptionalReceive receive(const ByteBufferView &buffer) {
      OptionalReceive received;
      if (buffer.empty()) return received;

      received.enabled = received->read(buffer) && received->check(); // TODO: handle errors
      return received;
    }

    bool send(const ByteBufferView &payload, DataUnitTypeCode type = DataUnitType::Bytes::Buffer) {
      if (payload.size() > Datagram::kPayloadSizeLimit) return false;

      Datagram datagram;
      datagram.header.type = type;
      return datagram.write(payload) && send(datagram);
    }

    // DatagramLink interface

    bool send(const Datagram &datagram) {
      datagram.writeHeader();
      return sender(datagram.buffer(), Datagram::kType); // TODO: handle error
    }
  
  protected:
    const ToSendDelegate &sender;
};

class ValidatedDatagramLink {
  public:
    using ToReceive = ByteBufferView; // The type of data passed up from below
    using Receive = ValidatedDatagram; // The type of data passed up to above
    using OptionalReceive = Util::Optional<Receive>;
    using Send = ByteBufferView; // The type of data passed down from above
    using SendDelegate = etl::delegate<bool(const Send &, DataUnitTypeCode)>;
    using ToSend = ByteBufferView; // The type of data passed down to below
    using ToSendDelegate = etl::delegate<bool(const ToSend &, DataUnitTypeCode)>;

    ValidatedDatagramLink(const ToSendDelegate &delegate) : sender(delegate) {}
    ValidatedDatagramLink(const ValidatedDatagramLink &link) = delete; // prevent accidental copy-by-value

    // Event loop interface

    void setup() {}
    void update() {}

    // ByteBufferLink interface

    OptionalReceive receive(const ByteBufferView &buffer, DataUnitTypeCode type) {
      OptionalReceive received;
      if ((
        type != DataUnitType::Bytes::Buffer
        && type != DataUnitType::Transport::ValidatedDatagram
      ) || buffer.empty()) return received;
      
      if (!received->read(buffer)) return received;

      //received.enabled = received->read(buffer) && received->check(); // TODO: handle errors
      received.enabled = received->check(); // TODO: handle errors
      return received;
    }

    bool send(const ByteBufferView &payload, DataUnitTypeCode type = DataUnitType::Bytes::Buffer) {
      if (payload.size() > ValidatedDatagram::kPayloadSizeLimit) return false;

      ValidatedDatagram datagram;
      datagram.header.type = type;
      return datagram.write(payload) && send(datagram); // TODO: handle error
    }

    // DatagramLink interface

    bool send(const ValidatedDatagram &datagram) {
      datagram.writeHeader();
      return sender(datagram.buffer(), ValidatedDatagram::kType); // TODO: handle error
    }
  
  protected:
    const ToSendDelegate &sender;
};

} } }