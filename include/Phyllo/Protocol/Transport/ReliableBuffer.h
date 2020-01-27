#pragma once

// Standard libraries

// Third-party libraries
#include <etl/algorithm.h>

// Phyllo
#include "Phyllo/Types.h"
#include "Phyllo/Util/Struct.h"
#include "Datagram.h"

// ReliableBuffers are used to send discrete units of data reliably over a validated datagram link
// in which datagrams are vulnerable to loss (data corruption detectable by CRCs in validated datagrams)

namespace Phyllo { namespace Protocol { namespace Transport {

class ReliableBufferFlags {
  public:
    using Bitfield = uint8_t;

    template<size_t Position>
    using Flag = Util::BitfieldFlag<Bitfield, Position>;

    Flag<0> fin; // last reliableBuffer from sender
    Flag<1> syn; // synchronize sequence numbers (first reliableBuffer from sender)
    Flag<2> nos; // ignore the sequence number field (reliableBuffer in unreliable transmission mode)
    Flag<3> ack; // acknowledgement number field is significant
    Flag<4> nak; // negative acknowledgement (to request resend of all in-flight reliableBuffers); examined only if ack is set
    Flag<5> sak; // selective acknowledgement (to treat the acknowledgement number as selective instead of cumulative)
    Flag<6> rst; // reset the connection
    Flag<7> ext; // extended header (treat part of the payload as actually being a header extension)

    ReliableBufferFlags() {} // default constructor leaves all flags false
    ReliableBufferFlags(Bitfield bitfield) : // implicit conversion from Bitfield
      fin(bitfield),
      syn(bitfield),
      nos(bitfield),
      ack(bitfield),
      nak(bitfield),
      sak(bitfield),
      rst(bitfield),
      ext(bitfield) {}


    operator Bitfield() const { // implicit conversion to Bitfield
      return (
        Bitfield(0)
        | fin.bitfield()
        | syn.bitfield()
        | nos.bitfield()
        | ack.bitfield()
        | nak.bitfield()
        | sak.bitfield()
        | rst.bitfield()
        | ext.bitfield()
      );
    }
};

class ReliableBufferHeader {
  public:
    using SequenceNumber = uint8_t;

    using SeqNumField = Util::StructField<SequenceNumber, 0>; // 1 byte
    using AckNumField = Util::StructField<SequenceNumber, SeqNumField::kAfterOffset>; // 1 byte; expected sequence number of next reliableBuffer to be received
    using FlagsField = Util::StructField<ReliableBufferFlags, AckNumField::kAfterOffset, ReliableBufferFlags::Bitfield>; // 1 byte
    using TypeField = Util::StructField<DataUnitTypeCode, FlagsField::kAfterOffset>; // 1 byte

    static const size_t kSize = (
      0
      + SeqNumField::kSize
      + AckNumField::kSize
      + FlagsField::kSize
      + TypeField::kSize
    );

    SeqNumField seqNum = 0;
    AckNumField ackNum = 0;
    FlagsField flags;
    TypeField type = DataUnitType::Bytes::Buffer;

    bool read(const ByteBufferView &buffer) {
      if (buffer.size() < kSize) return false; // TODO: handle error

      seqNum.read(buffer);
      ackNum.read(buffer);
      flags.read(buffer);
      type.read(buffer);
      return true;
    }

    bool write(ByteBuffer &buffer) {
      if (buffer.size() < kSize) return false;

      seqNum.write(buffer);
      ackNum.write(buffer);
      flags.write(buffer);
      type.write(buffer);
      return true;
    };
};

class ReliableBuffer {
  public:
    static const DataUnitTypeCode kType = DataUnitType::Transport::ReliableBuffer;
    static const size_t kHeaderSize = ReliableBufferHeader::kSize;
    static const size_t kFooterSize = 0;
    static const size_t kOverheadSize = kHeaderSize + kFooterSize;
    static const size_t kPayloadSizeLimit = Datagram::kPayloadSizeLimit - kOverheadSize;

    ReliableBufferHeader header;

    ReliableBuffer() {}

    ByteBufferView payload() const {
      return ByteBufferView(dumpBuffer.begin() + kHeaderSize, dumpBuffer.end() - kFooterSize);
    }
    ByteBufferView buffer() const {
      return ByteBufferView(dumpBuffer);
    }

    // Methods for updating reliableBuffer or reliableBuffer payload

    bool read(const ByteBufferView &buffer) {
      // Parse a given payload, update the own header and payload, and dump to own buffer
      if (buffer.size() < kOverheadSize) return false; // TODO: handle this as a datagram-level error signal in DatagramLink
      if (!header.read(buffer)) return false;

      // Dump payload and header into own buffer
      ByteBufferView payload(buffer.begin() + kHeaderSize, buffer.end() - kFooterSize);

      return dump(payload);
    }

    bool write(const ByteBufferView &payload) {
      // Write a payload, update the header for consistency, and dump to own buffer
      if (payload.empty()) return false;
      if (payload.size() > kPayloadSizeLimit) return false;

      return dump(payload);
    }

    bool writeEmpty() {
      // Write an empty payload, update the header for consistency, and dump to own buffer
      dumpBuffer.resize(kOverheadSize);
      return header.write(dumpBuffer);
    }

    ReliableBuffer &operator=(const ReliableBuffer &reliableBuffer) {
      header = reliableBuffer.header;
      dumpBuffer.resize(reliableBuffer.buffer().size());
      memcpy(dumpBuffer.data(), reliableBuffer.buffer().data(), reliableBuffer.buffer().size());
      return *this;
    }

  protected:
    using DumpBuffer = FixedByteBuffer<Datagram::kPayloadSizeLimit>;
    DumpBuffer dumpBuffer;

    bool dump(const ByteBufferView &payload) {
      dumpBuffer.resize(kOverheadSize + payload.size());
      memcpy(dumpBuffer.begin() + kHeaderSize, payload.data(), payload.size());
      
      return header.write(dumpBuffer);
    }
};

} } }

namespace Phyllo {

ByteBufferView getPayload(const Protocol::Transport::ReliableBuffer &reliable) {
    return reliable.payload();
}
Protocol::DataUnitTypeCode getPayloadType(const Protocol::Transport::ReliableBuffer &reliable) {
    return reliable.header.type;
}

}