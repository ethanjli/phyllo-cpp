#pragma once

// Standard libraries

// Third-party libraries

// Phyllo

// Bitfields are used for storage of boolean arrays in a compact way

namespace Phyllo { namespace Util {

// Bitfield manipulations, copied from http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2014/n3864.html
// Modified so that position is size_t instead of int.
template<typename integral>
constexpr integral write(bool value, size_t position) noexcept {
  return integral(value) << position;
}
template<typename integral>
constexpr integral mask(size_t position) noexcept {
  return integral(true) << position;
}
template<typename integral>
constexpr integral setbit(integral x, size_t position) noexcept {
  return x | mask<integral>(position);
}
template<typename integral>
constexpr integral resetbit(integral x, size_t position) noexcept {
  return x & ~mask<integral>(position);
}
template<typename integral>
constexpr integral flipbit(integral x, size_t position) noexcept {
  return x ^ mask<integral>(position);
}
template<typename integral>
constexpr bool testbit(integral x, size_t position) noexcept {
  return bool(x & mask<integral>(position));
}

} }