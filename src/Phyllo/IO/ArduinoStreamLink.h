#pragma once

// Standard libraries
#include <Arduino.h>

// Third-party libraries

// Phyllo
#include "Phyllo/Protocol/Transport/StreamLink.h"
#include "Phyllo/Types.h"
#include "Phyllo/Protocol/Types.h"

// Frame layer handles data framing

namespace Phyllo { namespace Protocol { namespace Transport {

template<>
int StreamLink<Stream>::hasRead() {
  return stream->available();
}

template<>
StreamLink<Stream>::Receive StreamLink<Stream>::peek() const {
  return stream->peek();
}

template<>
void StreamLink<Stream>::consume() {
  stream->read();
}

template<>
StreamLink<Stream>::Receive StreamLink<Stream>::read() {
  return stream->read();
}

template<>
bool StreamLink<Stream>::send(const ByteBufferView &buffer, uint8_t type) {
  stream->write(buffer.data(), buffer.size());
  return true;
}
template<>
bool StreamLink<Stream>::send(const uint8_t *buffer, size_t size, uint8_t type) {
  if (buffer == nullptr || !size) return false;

  stream->write(buffer, size);
  return true;
}
template<>
bool StreamLink<Stream>::send(uint8_t byte, uint8_t type) {
  stream->write(byte);
  return true;
}

template<>
size_t StreamLink<Stream>::read(Receive *buffer, size_t maxLength) {
  return stream->readBytes(buffer, maxLength);
}

template<>
void StreamLink<Stream>::setTimeout() {
  stream->setTimeout(kTimeout);
}

} } }