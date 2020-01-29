#pragma once

// Standard libraries

// Third-party libraries
#include <etl/delegate.h>

// Phyllo
#include "Phyllo/Types.h"
#include "Phyllo/Util/Optional.h"
#include "Message.h"
#include "Document.h"

// Document layer handles document serialization and deserialization. Documents are anything which can be
// represented in JSON: regular arrays (like Python lists or tuples), associative arrays (like Python dicts),
// and primitives (like numbers or strings).

namespace Phyllo { namespace Protocol { namespace Application { namespace PubSub {

template<Presentation::SerializationFormatCode Format>
class DocumentLink {
  public:
    using Topic = ByteBufferView;
    using ToReceive = ByteBufferView; // The type of data passed up from below
    using Receive = Document<Format>; // The type of data passed up to above
    using OptionalReceive = Util::Optional<Receive>;
    using Send = Document<Format>;
    using SendDelegate = etl::delegate<bool(const Send &)>;
    using ToSend = ByteBufferView; // The type of data passed down to below
    using ToSendDelegate = etl::delegate<bool(const Topic &, const ToSend &, DataUnitTypeCode)>;

    DocumentLink(const ToSendDelegate &delegate) : sender(delegate) {}
    DocumentLink(const DocumentLink &documentLink) = delete; // prevent accidental copy-by-value

    // Event loop interface

    void setup() {}
    void update() {}

    // DocumentLink interface

    OptionalReceive receive(const ByteBufferView &topic, const ByteBufferView &buffer) {
      OptionalReceive received;
      if (buffer.empty()) return received;

      received.enabled = received->read(buffer); // TODO: handle errors
      received->setTopic(topic);
      return received;
    }

    template<typename Class>
    typename etl::enable_if<!etl::is_one_of<Class, Send, Presentation::Document<Format>, ByteBuffer, ByteBufferView>::value, bool>::type
    send(const ByteBufferView &topic, Class &instance) {
      Document<Format> document;
      document.header.schema = Class::kSchema;
      document.setTopic(topic);
      if (!document.write(instance)) return false; // TODO: errors in payload writing must propagate up to the document! MessageReader needs to return whether it succeeded at the end of parsing, and readClass needs to check for errors from the class method for reading
      return send(document);
    }
    template<typename Class>
    typename etl::enable_if<!etl::is_one_of<Class, Send, Presentation::Document<Format>, ByteBuffer, ByteBufferView>::value, bool>::type
    send(const ByteBufferView &topic, const Class &instance) {
      return send(topic, static_cast<Class &>(instance));
    }
    bool send(const ByteBufferView &topic, const Presentation::Document<Format> &document) {
      return sender(topic, document.buffer(), Send::kType);
    }
    bool send(const Send &document) {
      return sender(document.topic(), document.buffer(), Send::kType);
    }
    bool send(
      const ByteBufferView &topic, const ByteBufferView &body,
      Presentation::SchemaCode schema = Presentation::Schema::Generic::Schemaless
    ) {
      Send document;
      document.header.schema = schema;
      document.setTopic(topic);
      return document.write(body) && send(document);
    }

  protected:
    const ToSendDelegate &sender;
};

} } } }