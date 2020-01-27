#pragma once

// Standard libraries

// Third-party libraries
#include <etl/endianness.h>

// Phyllo

// Endian-correctness for working with network data

namespace Phyllo { namespace Util {

template<typename Number>
Number readFromNetwork(const uint8_t * const buffer) { // reads data from network order representation and returns in host order
  if (buffer == nullptr) return 0;

  Number number;
  //Number number = *reinterpret_cast<const Number *>(buffer); // This causes a failure with uint32_t on Teensy LC - maybe there are alignment issues?
  memcpy(&number, buffer, sizeof(Number)); // This seems to work everywhere
  return etl::ntoh(number);
}

template<>
uint8_t readFromNetwork<uint8_t>(const uint8_t * const buffer) { // reads data from network order representation and returns in host order
  if (buffer == nullptr) return 0;

  return *buffer;
}

template<typename Number>
void writeToNetwork(Number number, uint8_t *buffer) { // writes data from host order to network order representation
  if (buffer == nullptr) return;

  //*reinterpret_cast<Number *>(buffer) = etl::hton(number); // no problems encountered, but reinterpret_cast seems to be risky
  number = etl::hton(number);
  memcpy(buffer, &number, sizeof(Number));
}

template<>
void writeToNetwork<uint8_t>(uint8_t number, uint8_t *buffer) { // writes data from host order to network order representation
  if (buffer == nullptr) return;

  *buffer = number;
}

} }