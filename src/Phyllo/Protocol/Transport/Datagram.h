#pragma once

// Standard libraries

// Third-party libraries
#include <etl/algorithm.h>
#if PHYLLO_PLATFORM == PHYLLO_PLATFORM_ATMELAVR
#define ETL_CPP11_SUPPORTED 0 // Needed for proper ETL compilation above v16.3.1
#endif
#include <etl/optional.h>

// Phyllo
#include "Phyllo/Types.h"
#include "Phyllo/Util/CRC.h"
#include "Phyllo/Util/Struct.h"
#include "Phyllo/Protocol/Types.h"
#include "FrameLink.h"

// Datagrams are transmitted without reliability guarantees, though their headers' length and CRC fields
// can be used to check header and payload validity.

namespace Phyllo { namespace Protocol { namespace Transport {

class DatagramHeader {
  public:
    using Length = uint8_t;

    using LengthField = Util::StructField<Length, 0>; // 1 byte
    using TypeField = Util::StructField<DataUnitTypeCode, LengthField::kAfterOffset>; // 1 byte

    static const size_t kSize = LengthField::kSize + TypeField::kSize;

    LengthField length = 0;
    TypeField type = DataUnitType::Bytes::Buffer;

    bool read(const ByteBufferView &buffer) {
      if (buffer.size() < kSize) return false; // TODO: handle error

      length.read(buffer);
      type.read(buffer);
      return true;
    }

    bool write(ByteBuffer &buffer) const {
      if (buffer.size() < kSize) return false;

      length.write(buffer);
      type.write(buffer);
      return true;
    };
};

class Datagram {
  public:
    static const DataUnitTypeCode kType = DataUnitType::Transport::Datagram;
    static const size_t kHeaderSize = DatagramHeader::kSize;
    static const size_t kFooterSize = 0;
    static const size_t kOverheadSize = kHeaderSize + kFooterSize;
    static const size_t kPayloadSizeLimit = FrameLink::kPayloadSizeLimit - kOverheadSize;

    DatagramHeader header;

    Datagram() {}

    ByteBufferView payload() const {
      return ByteBufferView(dumpBuffer.begin() + kHeaderSize, dumpBuffer.end() - kFooterSize);
    }
    ByteBufferView buffer() const {
      return ByteBufferView(dumpBuffer);
    }

    // Methods for consistency between header and own buffer

    bool update() {
      // Update own header and buffer for consistency with own payload
      header.length = static_cast<uint8_t>(getPayloadLength());
      return writeHeader();
    }

    bool check() const {
      // Check consistency between header and own buffer
      return header.length == getPayloadLength();
    }

    // Methods for updating datagram or payload

    bool read(const ByteBufferView &buffer) {
      // Parse a given buffer, update the own header and payload, and dump to own buffer. Does not enforce header consistency.
      if (buffer.size() < kOverheadSize) return false; // TODO: handle this as an error signal
      if (!header.read(buffer)) return false;

      // Dump payload and header into own buffer
      ByteBufferView payload(buffer.begin() + kHeaderSize, buffer.end() - kFooterSize);

      return dump(payload);
    }

    bool write(const ByteBufferView &payload, bool update = true) {
      // Write a payload, update the header for consistency, and dump to own buffer
      if (payload.empty()) return false;
      if (payload.size() > kPayloadSizeLimit) return false;

      if (!dump(payload)) return false;
      if (update) return this->update();

      return true;
    }
    bool writeHeader() const {
      // Dump header to own buffer. This is necessary if the document is written using DocumentWriter!
      return header.write(const_cast<DumpBuffer &>(dumpBuffer));
    }
    bool writeEmpty(bool update = true) {
      // Write an empty payload, update the header for consistency, and dump to own buffer
      dumpBuffer.resize(kHeaderSize);
      if (update) return this->update();
      else return writeHeader();
    }

    Datagram &operator=(const Datagram &datagram) {
      header = datagram.header;
      dumpBuffer.resize(datagram.buffer().size());
      memcpy(dumpBuffer.data(), datagram.buffer().data(), datagram.buffer().size());
      return *this;
    }

  protected:
    using DumpBuffer = FixedByteBuffer<FrameLink::kPayloadSizeLimit>;
    DumpBuffer dumpBuffer;

    bool dump(const ByteBufferView &payload) {
      dumpBuffer.resize(kOverheadSize + payload.size());
      memcpy(dumpBuffer.begin() + kHeaderSize, payload.data(), payload.size());

      return header.write(dumpBuffer);
    }

    size_t getPayloadLength() const {
      return dumpBuffer.size() - kOverheadSize;
    }
};

} } }

namespace Phyllo {

ByteBufferView getPayload(const Protocol::Transport::Datagram &datagram) {
    return datagram.payload();
}
Protocol::DataUnitTypeCode getPayloadType(const Protocol::Transport::Datagram &datagram) {
    return datagram.header.type;
}

}

namespace Phyllo { namespace Protocol { namespace Transport {

class ValidatedDatagramHeader {
  public:
    using Length = uint8_t;

    using CRCField = Util::StructField<Util::CRC, 0>; // 4 bytes
    using TypeField = Util::StructField<DataUnitTypeCode, CRCField::kAfterOffset>; // 1 byte

    static const size_t kProtectedOffset = CRCField::kAfterOffset;
    static const size_t kProtectedSize = TypeField::kSize;
    static const size_t kSize = CRCField::kSize + kProtectedSize;

    CRCField crc = 0;
    TypeField type = DataUnitType::Bytes::Buffer;

    bool read(const ByteBufferView &buffer) {
      if (buffer.size() < kSize) return false;

      crc.read(buffer);
      type.read(buffer);
      return true;
    }

    bool writeCRC(ByteBuffer &buffer) const {
      if (buffer.size() < CRCField::kAfterOffset) return false;

      crc.write(buffer);
      return true;
    }
    bool writeProtected(ByteBuffer &buffer) const {
      if (buffer.size() < kSize) return false;

      type.write(buffer);
      return true;
    };

    bool write(ByteBuffer &buffer) const {
      return writeProtected(buffer) && writeCRC(buffer);
    }
};

class ValidatedDatagram {
  public:
    using CRC = Util::CRC;
    static const DataUnitTypeCode kType = DataUnitType::Transport::ValidatedDatagram;
    static const size_t kHeaderSize = ValidatedDatagramHeader::kSize;
    static const size_t kFooterSize = 0;
    static const size_t kOverheadSize = kHeaderSize + kFooterSize;
    static const size_t kPayloadSizeLimit = FrameLink::kPayloadSizeLimit - kOverheadSize;

    ValidatedDatagramHeader header;

    ValidatedDatagram() {}

    ByteBufferView payload() const {
      return ByteBufferView(dumpBuffer.begin() + kHeaderSize, dumpBuffer.end() - kFooterSize);
    }
    ByteBufferView buffer() const {
      return ByteBufferView(dumpBuffer);
    }

    // Methods for consistency between header and own buffer

    bool updateCRC() {
      // Update own header and buffer for consistency with own payload
      if (!cachedCRC) cachedCRC = computeCRC();
      header.crc = cachedCRC.value();
      return writeHeader();
    }

    bool check() const {
      // Check consistency between header and own buffer
      if (!cachedCRC) cachedCRC = computeCRC();
      return header.crc == cachedCRC.value();
    }

    // Methods for updating validated datagram or

    bool read(const ByteBufferView &buffer) {
      // Parse a given buffer, update the own header and payload, and dump to own buffer. Does not enforce header consistency.
      if (buffer.size() < kOverheadSize) return false; // TODO: handle this as an error signal
      if (!header.read(buffer)) return false;

      // Dump payload and header into own buffer to enable CRC consistency check
      ByteBufferView payload(buffer.begin() + kHeaderSize, buffer.end() - kFooterSize);

      return dump(payload);
    }

    bool write(const ByteBufferView &payload, bool update = true) {
      // Write a payload, update the header for consistency, and dump to own buffer
      if (payload.empty()) return false;
      if (payload.size() > kPayloadSizeLimit) return false;

      if (!dump(payload)) return false;
      if (update) return updateCRC();

      return true;
    }
    bool writeHeader() const {
      // Dump header to own buffer. This is necessary if the document is written using DocumentWriter!
      return header.write(const_cast<DumpBuffer &>(dumpBuffer));
    }
    bool writeEmpty(bool update = true) {
      // Write an empty payload, update the header for consistency, and dump to own buffer
      dumpBuffer.resize(kHeaderSize);
      cachedCRC = etl::nullopt;
      if (update) return updateCRC();
      else return writeHeader();
    }

    ValidatedDatagram &operator=(const ValidatedDatagram &validatedDatagram) {
      header = validatedDatagram.header;
      dumpBuffer.resize(validatedDatagram.buffer().size());
      memcpy(dumpBuffer.data(), validatedDatagram.buffer().data(), validatedDatagram.buffer().size());
      return *this;
    }

  protected:
    using DumpBuffer = FixedByteBuffer<FrameLink::kPayloadSizeLimit>;
    DumpBuffer dumpBuffer;
    mutable etl::optional<CRC> cachedCRC;

    bool dump(const ByteBufferView &payload) {
      dumpBuffer.resize(kOverheadSize + payload.size());
      memcpy(dumpBuffer.begin() + kHeaderSize, payload.begin(), payload.size());

      cachedCRC = etl::nullopt;
      return header.write(dumpBuffer);
    }

    CRC computeCRC() const {
      // Compute from protected section of own buffer
      return Util::reflectedCRC32sub8(
        dumpBuffer.data() + ValidatedDatagramHeader::kProtectedOffset,
        ValidatedDatagramHeader::kProtectedSize + getPayloadLength() + kFooterSize
      );
    }

    size_t getPayloadLength() const {
      return dumpBuffer.size() - kOverheadSize;
    }
};

} } }

namespace Phyllo {

ByteBufferView getPayload(const Protocol::Transport::ValidatedDatagram &validated) {
    return validated.payload();
}
Protocol::DataUnitTypeCode getPayloadType(const Protocol::Transport::ValidatedDatagram &validated) {
    return validated.header.type;
}

}