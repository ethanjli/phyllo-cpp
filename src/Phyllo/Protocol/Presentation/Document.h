#pragma once

// Note: a file with template specializations of DocumentReader and DocumentWriter,
// such as MessagePack.h, must be included before or after this file is included
// in order for Documents to be instantiatable.

// Standard libraries

// Third-party libraries

// Phyllo
#include "Phyllo/Util/Struct.h"
#include "Phyllo/Protocol/Transport/ReliableBufferLink.h"
#include "Types.h"

// Document layer handles document serialization and deserialization. Documents are anything which can be
// represented in JSON: regular arrays (like Python lists or tuples), associative arrays (like Python dicts),
// and primitives (like numbers or strings). Each document is associated at compile time with a specific
// serialization format.

namespace Phyllo { namespace Protocol { namespace Presentation {

class DocumentHeader {
  public:
    using Format = SerializationFormatCode;
    using Schema = SchemaCode;

    using FormatField = Util::StructField<Format, 0>; // 1 byte
    using SchemaField = Util::StructField<Schema, FormatField::kAfterOffset>; // 1 byte

    static const size_t kSize = FormatField::kSize + SchemaField::kSize;

    FormatField format = SerializationFormat::Binary::Dynamic::Unknown;
    SchemaField schema = Schema::Generic::Schemaless;

    bool read(const ByteBufferView &buffer) {
      if (buffer.size() < kSize) return false; // TODO: handle error

      format.read(buffer);
      schema.read(buffer);
      return true;
    }

    bool write(ByteBuffer &buffer) const {
      if (buffer.size() < kSize) return false;

      format.write(buffer);
      schema.write(buffer);
      return true;
    };
};

template<SerializationFormatCode Format>
class DocumentReader;

template<SerializationFormatCode Format>
class DocumentWriter;

template<SerializationFormatCode Format>
class Document {
  public:
    static const DataUnitTypeCode kType = DataUnitType::Presentation::Document;
    static const size_t kHeaderSize = DocumentHeader::kSize;
    static const size_t kFooterSize = 0;
    static const size_t kOverheadSize = kHeaderSize + kFooterSize;
    static const size_t kBodySizeLimit = Transport::ReliableBuffer::kPayloadSizeLimit - kOverheadSize;
    static const SerializationFormatCode kFormat = SerializationFormat::Binary::Dynamic::MsgPack;

    using Reader = DocumentReader<Format>;
    using Writer = DocumentWriter<Format>;

    DocumentHeader header;
    mutable Reader reader; // Warning: mutating the reader is not considered to break Document constness!
    mutable Writer writer; // Warning: mutating the writer is not considered to break Document constness!

    Document() :
      reader(dumpBuffer, kHeaderSize, kFooterSize),
      writer(dumpBuffer, header, kHeaderSize, kFooterSize) {
        header.format = Format;
        header.schema = Schema::Generic::Schemaless;
      }

    ByteBufferView body() const {
      return ByteBufferView(dumpBuffer.begin() + kHeaderSize, dumpBuffer.end() - kFooterSize);
    }

    ByteBufferView buffer() const {
      return ByteBufferView(dumpBuffer);
    }

    // Methods for updating document or body

    bool read(const ByteBufferView &buffer) { // deserialize a buffer into the header and payload buffer
      // Parse a given buffer, update the own header and body, and dump to own buffer
      if (buffer.size() < kOverheadSize) return false; // TODO: handle this as an error signal
      if (!header.read(buffer)) return false;

      // Dump body and header into own buffer
      ByteBufferView body(buffer.begin() + kHeaderSize, buffer.end() - kFooterSize);

      return dump(body);
    }
    template<typename Class>
    bool readPayload(Class &instance) const { // deserialize the payload buffer into an object
      reader.start();
      bool status = reader.readClass(instance);
      return reader.finish() && status;
    }

    template<typename Class>
    typename etl::enable_if<!etl::is_one_of<Class, ByteBuffer, ByteBufferView>::value, bool>::type
    write(const Class &instance) {
      writer.start(); // This comes before writeHeader because it also resizes dumpBuffer so that header can write into it!
      bool status = header.write(dumpBuffer);
      status = status && writer.writeClass(instance);
      return writer.finish() && status;
    }
    bool write(const ByteBufferView &body) {
      // Write a body, update the header for consistency, and dump to own buffer
      if (body.empty()) return false;
      if (body.size() > kBodySizeLimit) return false;
      return dump(body);
    }
    bool writeEmpty() {
      // Write an empty body, and dump to own buffer
      dumpBuffer.resize(kHeaderSize);
      return header.write(dumpBuffer);
    }

    Document &operator=(const Document &document) {
      header = document.header;
      dumpBuffer.resize(document.buffer().size());
      memcpy(dumpBuffer.data(), document.buffer().data(), document.buffer().size());
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
};

} } }

namespace Phyllo {

template<Protocol::Presentation::SerializationFormatCode Format>
Protocol::DataUnitTypeCode getSchema(const Protocol::Presentation::Document<Format> &document) {
    return document.header.schema;
}

}