#pragma once

// Note: a file with template specializations of DocumentReader and DocumentWriter,
// such as MessagePack.h, must be included before or after this file is included
// in order for Documents to be instantiatable.

// Standard libraries

// Third-party libraries

// Phyllo
#include "Phyllo/Util/Struct.h"
#include "Phyllo/Protocol/Types.h"
#include "Phyllo/Protocol/Presentation/Types.h"

// Document layer handles document serialization and deserialization. Documents are anything which can be
// represented in JSON: regular arrays (like Python lists or tuples), associative arrays (like Python dicts),
// and primitives (like numbers or strings). Each document is associated at compile time with a specific
// serialization format.

namespace Phyllo { namespace Protocol { namespace Application { namespace PubSub {

template<Presentation::SerializationFormatCode Format>
class Document : public Presentation::Document<Format> {
  public:
    Document() {}

    ByteBufferView topic() const {
      return ByteBufferView(messageTopic);
    }

    bool setTopic(const ByteBufferView &topic) {
      if (topic.size() > messageTopic.max_size()) return false;
      messageTopic.resize(topic.size());
      memcpy(messageTopic.data(), topic.data(), topic.size());
      return true;
    }
  
  protected:
    Binary16 messageTopic;
};

} } } }

namespace Phyllo {

template<Protocol::Presentation::SerializationFormatCode Format>
ByteBufferView getTopic(const Protocol::Application::PubSub::Document<Format> &document) {
    return document.topic();
}
template<Protocol::Presentation::SerializationFormatCode Format>
ByteBufferView getBody(const Protocol::Application::PubSub::Document<Format> &document) {
    return document.body();
}
template<Protocol::Presentation::SerializationFormatCode Format>
Protocol::Presentation::SchemaCode getPayloadSchema(const Protocol::Application::PubSub::Document<Format> &document) {
    return document.header.schema;
}

}