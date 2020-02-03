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

/*using VerbCode = uint8_t;

namespace Verb { // Like an enum, but extensible and less strict
  // Allowed values: 0x00 - 0x7f
  // Basic:
  static const VerbCode call  = 0x01;
  static const VerbCode read  = 0x02;
  static const VerbCode write = 0x03;
  static const VerbCode dir   = 0x04;
  static const VerbCode info  = 0x11;
  static const VerbCode chunk = 0x12;
  static const VerbCode await = 0x13;
  static const VerbCode error = 0x21;
  static const VerbCode warn  = 0x22;
  static const VerbCode none  = 0x23;
  // Push Notifications:
  static const VerbCode sub   = 0x31;
  static const VerbCode unsub = 0x32;
  static const VerbCode push  = 0x33;
}

class Command {
  public:
    using NamePath = const char *;
    using Id = unsigned long;

    // Only use setters to change these!
    VerbCode verb;
    NamePath name;
    Id id;
    JsonVariant args;

    Command() {
      this->document.clear();
      document.add(Verb::none);
      document.add(kNone);
      document.add(0);
      document.add(kNone);
      updateMembers();
    }

    Command(const Document &document) {
      this->document.clear();
      this->document.add(document[0]);
      this->document.add(document[1]);
      this->document.add(document[2]);
      this->document.add(document[3]);
      updateMembers();
    }

    template<typename Args>
    Command(VerbCode verb, const NamePath &name, Id id, Args args) {
      document.clear();
      document.add(verb);
      document.add(name);
      document.add(id);
      document.add(args);
      updateMembers();
    }

    Command(VerbCode verb, const NamePath &name, Id id) :
      Command(verb, name, id, kNone) {}

    Command(const Command &command) :
      Command(command.verb, command.name, command.id, command.args) {}

    operator Document() const { // implicit conversion to Document
      return document;
    }

    // Setters

    void setVerb(VerbCode verb) {
      document[0] = verb;
      this->verb = verb;
    }

    void setName(const NamePath &name) { // copy the name into own memory
      document[1] = name;
      this->name = document[1];
    }

    void setId(Id id) {
      document[2] = id;
      this->id = id;
    }

    template<typename Args>
    void setArgs(const Args &args) { // copy the args into own memory
      document[3] = args;
      this->args = document[3];
    }

    template<typename Args>
    void set(VerbCode verb, const NamePath &name, Id id, Args args = kNone) {
      document[0] = verb;
      document[1] = name;
      document[2] = id;
      document[3] = args;
      updateMembers();
    }

  protected:
    Document document;

    void updateMembers() {
      verb = document[0];
      name = document[1];
      id = document[2];
      args = document[3];
    }
};

class CommandLink {
  public:
    DocumentLink &documentLink;

    Command receivedCommand;
    bool received = false;

    CommandLink(DocumentLink &documentLink) : documentLink(documentLink) {}

    void setup() {
      documentLink.setup();
    }

    void update() {
      received = false;
      documentLink.update();

      if (!documentLink.received) return;

      receivedCommand = documentLink.receivedPayload;
      received = true;
    }

    void sendCommand(const Command &command, PacketFlags::Bitfield packetFlags = 0) {
      documentLink.sendDocument(command, packetFlags);
    }
};*/

} } } }

namespace Phyllo {

/*template<Protocol::Presentation::SerializationFormatCode PayloadFormat>
ByteBufferView getTopic(
  const Protocol::Application::PubSub::Message<PayloadFormat> &message
) {
    return ByteBufferView(message.topic);
}
template<Protocol::Presentation::SerializationFormatCode PayloadFormat>
Protocol::Presentation::Document<PayloadFormat> getPayload(
  const Protocol::Application::PubSub::Message<PayloadFormat> &message
) {
    return message.payload;
}*/

}