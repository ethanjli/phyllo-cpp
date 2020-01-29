#pragma once

// Note: a file with template specializations of DocumentReader and DocumentWriter,
// such as MessagePack.h, must be included before or after this file is included
// in order for Documents to be instantiatable.

// Standard libraries

// Third-party libraries

// Phyllo
#include "Phyllo/Util/Struct.h"
#include "Phyllo/Protocol/Types.h"
#include "Phyllo/Protocol/Transport/ReliableBufferLink.h"

// Document layer handles document serialization and deserialization. Documents are anything which can be
// represented in JSON: regular arrays (like Python lists or tuples), associative arrays (like Python dicts),
// and primitives (like numbers or strings). Each document is associated at compile time with a specific
// serialization format.

namespace Phyllo { namespace Protocol { namespace Application { namespace PubSub {

/*template<Presentation::SerializationFormatCode PayloadFormat>
class Message {
  public:
    static const Presentation::SerializationFormatCode kFormat =
      Presentation::SerializationFormat::Binary::Dynamic::MsgPack;
    using MessageDocument = Presentation::Document<kFormat>;
    using MessageReader = Presentation::DocumentReader<kFormat>;
    using MessageWriter = Presentation::DocumentWriter<kFormat>;
    using PayloadDocument = Presentation::Document<PayloadFormat>;

    Binary16 topic;
    PayloadDocument payload;

    bool writeTopic(const ByteBufferView &topic) { // set the topic
      if (topic.size() > this->topic.max_size()) return false;
      this->topic.resize(topic.size());
      memcpy(this->topic.data(), topic.data(), topic.size());
      return true;
    }

    // Serializable Document interface

    bool read(MessageReader &reader) { // used by DocumentReader to deserialize from a MessagePack Document body
      reader.startArrayLength(2);
      reader.readBinary(topic);
      bool status = payload.read(reader.readBinaryView());
      reader.finishArray();
    
      return !reader.error() && status;
    }
    bool write(MessageWriter &writer) const { // used by DocumentWriter to serialize into a MessagePack Document body
      writer.startArray(2);
      writer.writeBinary(topic);
      writer.writeBinary(payload.buffer());
      writer.finishArray();
      return !writer.error();
    }
};*/

class MessageHeader {
  public:
    using Length = uint8_t;

    using TypeField = Util::StructField<DataUnitTypeCode, 0>; // 1 byte
    using TopicLengthField = Util::StructField<Length, TypeField::kAfterOffset>; // 1 byte

    static const size_t kSize = TypeField::kSize + TopicLengthField::kSize;

    TypeField type = DataUnitType::Bytes::Buffer;
    TopicLengthField topicLength = 0;

    bool read(const ByteBufferView &buffer) {
      if (buffer.size() < kSize) return false; // TODO: handle error

      type.read(buffer);
      topicLength.read(buffer);
      return true;
    }

    bool write(ByteBuffer &buffer) const {
      if (buffer.size() < kSize) return false;

      type.write(buffer);
      topicLength.write(buffer);
      return true;
    };
};

class Message {
  public:
    static const DataUnitTypeCode kType = DataUnitType::Application::PubSub;
    static const size_t kHeaderSize = MessageHeader::kSize;
    static const size_t kFooterSize = 0;
    static const size_t kOverheadSize = kHeaderSize + kFooterSize;
    static const size_t kTopicSizeLimit = 15;
    static const size_t kBodySizeLimit = Transport::ReliableBuffer::kPayloadSizeLimit - kOverheadSize;

    MessageHeader header;

    Message() {}

    ByteBufferView topic() const {
      return ByteBufferView(dumpBuffer.begin() + kHeaderSize, dumpBuffer.begin() + kHeaderSize + header.topicLength);
    }
    ByteBufferView payload() const {
      return ByteBufferView(dumpBuffer.begin() + kHeaderSize + header.topicLength, dumpBuffer.end() - kFooterSize);
    }
    ByteBufferView buffer() const {
      return ByteBufferView(dumpBuffer);
    }

    // Methods for updating document or body

    bool read(const ByteBufferView &buffer) { // deserialize a buffer into the header and payload buffer
      // Parse a given buffer, update the own header and body, and dump to own buffer
      if (buffer.empty()) return false;
      if (buffer.size() < kOverheadSize) return false; // TODO: handle this as an error signal
      
      if (!header.read(buffer)) return false;
      // Dump body and header into own buffer
      ByteBufferView body(buffer.begin() + kHeaderSize, buffer.end() - kFooterSize);

      return dump(body);
    }

    bool write(const ByteBufferView &topic, const ByteBufferView &payload) {
      // Write a payload, update the header for consistency, and dump to own buffer
      if (payload.empty()) return false;
      if (payload.size() + topic.size() > kBodySizeLimit) return false;
      if (topic.size() > kTopicSizeLimit) return false;

      dumpBuffer.resize(kOverheadSize + payload.size() + topic.size());
      header.topicLength = topic.size();
      memcpy(dumpBuffer.begin() + kHeaderSize, topic.data(), topic.size());
      memcpy(dumpBuffer.begin() + kHeaderSize + topic.size(), payload.data(), payload.size());
      return header.write(dumpBuffer);
    }

    Message &operator=(const Message &message) {
      header = message.header;
      dumpBuffer.resize(message.buffer().size());
      memcpy(dumpBuffer.data(), message.buffer().data(), message.buffer().size());
      return *this;
    }

  protected:
    using DumpBuffer = FixedByteBuffer<Transport::ReliableBuffer::kPayloadSizeLimit>;
    DumpBuffer dumpBuffer;

    bool dump(const ByteBufferView &body) {
      dumpBuffer.resize(kOverheadSize + body.size());
      memcpy(dumpBuffer.begin() + kHeaderSize, body.data(), body.size());

      return header.write(dumpBuffer);
    }

    size_t getBodyLength() const {
      return dumpBuffer.size() - kOverheadSize;
    }
    size_t getTopicLength() const {
      return header.topicLength;
    }
    size_t getPayloadLength() const {
      return getBodyLength() - header.topicLength;
    }
};

} } } }

namespace Phyllo {

ByteBufferView getTopic(const Protocol::Application::PubSub::Message &message) {
    return message.topic();
}
ByteBufferView getPayload(const Protocol::Application::PubSub::Message &message) {
    return message.payload();
}
Protocol::DataUnitTypeCode getPayloadType(const Protocol::Application::PubSub::Message &message) {
    return message.header.type;
}

}