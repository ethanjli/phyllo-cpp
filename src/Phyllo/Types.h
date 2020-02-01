#pragma once

// Standard libraries

// Third-party libraries
#include <etl/array_view.h>
#include <etl/vector.h>
#include <etl/cstring.h>

// Phyllo

namespace Phyllo {

template<size_t bufferSize>
using FixedByteBuffer = etl::vector<uint8_t, bufferSize>; // Statically allocated array of variable length and fixed size

using ByteBuffer = etl::ivector<uint8_t>; // Reference to a vector of any possible size
using ByteBufferView = etl::array_view<const uint8_t>; // Read-only view of any array-like or vector-like buffer

ByteBufferView getPayload(const ByteBuffer &buffer) {
  return ByteBufferView(buffer);
}
ByteBufferView getPayload(const ByteBufferView &buffer) {
  return buffer;
}

using None = nullptr_t;
static const None kNone = nullptr;
using String = etl::istring;
using StringView = etl::string_view;
template<size_t Size>
using FixedString = etl::string<Size>;
using String8 = etl::string<8>;
using String16 = etl::string<16>;
using String32 = etl::string<32>;
using String64 = etl::string<64>;
using Binary = ByteBuffer;
using BinaryView = ByteBufferView;
template<size_t Size>
using FixedBinary = FixedByteBuffer<Size>;
using Binary8 = FixedByteBuffer<8>;
using Binary16 = FixedByteBuffer<16>;
using Binary32 = FixedByteBuffer<32>;
using Binary64 = FixedByteBuffer<64>;

#define PHYLLO_GENERIC_TYPES \
  None, bool, \
  const None, const bool, \
  unsigned int, uint8_t, uint16_t, uint32_t, uint64_t, \
  const unsigned int, const uint8_t, const uint16_t, const uint32_t, const uint64_t, \
  int, int8_t, int16_t, int32_t, int64_t, \
  const int, const int8_t, const int16_t, const int32_t, const int64_t, \
  float, double, \
  const float, const double, \
  StringView, char *, String8, String16, String32, String64, \
  const StringView, const char *, const String8, const String16, const String32, const String64, \
  BinaryView, Binary8, Binary16, Binary32, Binary64, \
  const BinaryView, const Binary8, const Binary16, const Binary32, const Binary64

}