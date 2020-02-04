#pragma once

// Standard libraries

// Third-party libraries
#include <etl/delegate.h>

// Phyllo
#include "Phyllo/Types.h"
#include "Phyllo/Util/Optional.h"
#include "Phyllo/Protocol/Types.h"
#include "Phyllo/Protocol/Presentation/Document.h"
#include "Phyllo/Protocol/Presentation/MessagePack.h"
#include "Message.h"

// Pub-Sub Messaging Framework associates payloads (such as serialized documents)
// with independent named topics

namespace Phyllo { namespace Protocol { namespace Application { namespace PubSub {

class MessageLink {
  public:
    using ToReceive = ByteBufferView; // The type of data passed up from below
    using Receive = Message; // The type of data passed up to above
    using OptionalReceive = Util::Optional<Receive>;
    using Topic = ByteBufferView;
    using Send = ByteBufferView;
    using SendDelegate = etl::delegate<bool(const Topic &, const Send &, DataUnitTypeCode)>;
    using ToSend = ByteBufferView; // The type of data passed down to below
    using ToSendDelegate = etl::delegate<bool(const ToSend &, DataUnitTypeCode)>;

    MessageLink(const ToSendDelegate &delegate) : sender(delegate) {}
    MessageLink(const MessageLink &messageLink) = delete; // prevent accidental copy-by-value

    // Event loop interface

    void setup() {}
    void update() {}

    // Named-topic Document Link interface

    OptionalReceive receive(const ByteBufferView &buffer) {
      OptionalReceive received;
      if (buffer.empty()) return received;

      received.enabled = received->read(buffer);
      return received;
    }

    bool send(
      const ByteBufferView &topic, const ByteBufferView &payload,
      DataUnitTypeCode type = DataUnitType::Bytes::Buffer
    ) {
      if (topic.size() + payload.size() > Message::kBodySizeLimit) return false;

      Message message;
      message.header.type = type;
      return message.write(topic, payload) && send(message);
    }
    bool send(const Message &message) {
      return sender(message.buffer(), Message::kType);
    }

  protected:
    const ToSendDelegate &sender;
};

} } } }