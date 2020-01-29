#pragma once

// Standard libraries

// Third-party libraries

// Phyllo
#include "Phyllo/Types.h"
#include "Endian.h"
#include "Bitfield.h"

// Structs are used to read and write data for a network in a structured way

namespace Phyllo { namespace Util {

template<typename Type, size_t offset, typename BufferType = Type>
class StructField {
  public:
    using ValueType = Type;
    using FieldType = BufferType;

    Type value = 0;
    static const size_t kStartOffset = offset;
    static const size_t kSize = sizeof(BufferType);
    static const size_t kAfterOffset = offset + kSize;

    StructField(Type value) :
      value(value) {}
    StructField() :
      value() {}

    template<typename Buffer>
    static Type parse(const Buffer &buffer) {
      return readFromNetwork<BufferType>(&buffer[kStartOffset]);
    }

    template<typename Buffer>
    void read(const Buffer &buffer) {
      value = parse(buffer);
    }

    void write(ByteBuffer &buffer) const {
      writeToNetwork<BufferType>(value, &buffer[kStartOffset]);
    }

    operator Type() const {
      return value;
    }

    StructField &operator=(Type value) {
      this->value = value;
      return *this;
    }
};

template<typename Bitfield, size_t Position>
class BitfieldFlag {
  public:
    static const size_t kPosition = Position;
    bool value = false;

    BitfieldFlag() {}
    BitfieldFlag(Bitfield bitfield) : // get from Bitfield, for implicit conversion from Bitfield
      value(testbit(bitfield, kPosition)) {}

    Bitfield bitfield() const {
      return write<Bitfield>(value, kPosition);
    }

    operator bool() const { // implicit conversion to bool
      return value;
    }

    BitfieldFlag &operator=(bool value) {
      this->value = value;
      return *this;
    }
};

} }