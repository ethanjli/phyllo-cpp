#pragma once

// Standard libraries

// Third-party libraries

// Phyllo
#include <stdint.h>

namespace Phyllo { namespace Protocol {

using DataUnitTypeCode = uint8_t;

namespace DataUnitType { // Like an enum, but extensible and less strict
  // Allowed values: 0x00 - 0xff
  // All message-oriented layers have a 1 byte type field to allow for multiplexing/demultiplexing different message payload types:
  namespace Layer {
    static const DataUnitTypeCode Control      = 0x00;
    static const DataUnitTypeCode Version      = 0x01;
    static const DataUnitTypeCode Capabilities = 0x02;
    static const DataUnitTypeCode Error        = 0x03;
    static const DataUnitTypeCode Warn         = 0x04;
    static const DataUnitTypeCode Info         = 0x05;
    static const DataUnitTypeCode Debug        = 0x06;
    static const DataUnitTypeCode Trace        = 0x07;
    static const DataUnitTypeCode Metrics      = 0x08;
    // 0x09 - 0x0f are reserved for definition by the layer
  }
  namespace Bytes {
    static const DataUnitTypeCode Buffer = 0x10;
    static const DataUnitTypeCode Stream = 0x11;
    static const DataUnitTypeCode Chunk  = 0x12;
    // 0x13 - 0x1f are reserved for future use
  }
  namespace Transport {
    static const DataUnitTypeCode Frame             = 0x20;
    static const DataUnitTypeCode Datagram          = 0x21;
    static const DataUnitTypeCode ValidatedDatagram = 0x22;
    static const DataUnitTypeCode ReliableBuffer    = 0x23;
    static const DataUnitTypeCode PortedBuffer      = 0x24;
    // 0x25 - 0x2f are reserved for future use
    // 0x3* is available for byte buffer payloads representing ad hoc data units defined by bring-your-own transport layers:
  }
  namespace Presentation {
    static const DataUnitTypeCode Document = 0x40;
    // 0x41 - 0x4f are reserved for future use
    // 0x5* is available for byte buffer payloads representing ad hoc application-defined presentation-level serialized messages:
  }
  namespace Application {
    static const DataUnitTypeCode PubSub = 0x60;
    static const DataUnitTypeCode RPC    = 0x61;
    static const DataUnitTypeCode REST   = 0x62;
    // 0x64 - 0x6f are reserved for future use
  }
  // 0x7* - 0xff are reserved for future use.
}

} }