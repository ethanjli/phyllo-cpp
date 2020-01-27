#pragma once

// Standard libraries

// Third-party libraries

// Phyllo
#include "Phyllo/Protocol/Types.h"

namespace Phyllo { namespace Protocol { namespace Presentation {

using SerializationFormatCode = uint8_t;

namespace SerializationFormat { // Like an enum, but extensible and less strict
  // Allowed values: 0x00 - 0xff
  namespace Layer = DataUnitType::Layer;
  namespace Binary {
    namespace Dynamic {
      static const SerializationFormatCode Unknown = 0x10;
      static const SerializationFormatCode MsgPack = 0x11;
      static const SerializationFormatCode CBOR    = 0x12;
      static const SerializationFormatCode BSON    = 0x13;
      static const SerializationFormatCode Avro    = 0x14;
      // 0x05 - 0x0f are reserved for future use
      // 0x1* is available for ad hoc binary dynamically-typed serialization formats defined by programmers/applications
    }
    namespace Static {
      static const SerializationFormatCode Protobuf    = 0x30;
      static const SerializationFormatCode Thrift      = 0x31;
      static const SerializationFormatCode CapnProto   = 0x32;
      static const SerializationFormatCode FlatBuffers = 0x33;
      // 0x24 - 0x2f are reserved for future use
      // 0x3* is available for ad hoc binary statically-typed serialization formats defined by programmers/applications
    }
  }
  namespace Text {
    static const SerializationFormatCode JSON = 0x50;
    static const SerializationFormatCode CSV  = 0x51;
    // 0x44 - 0x4f are reserved for future use
    // 0x5* is available for ad hoc text-based serialization formats defined by programmers/applications
  }
  // 0x6* - 0xff are reserved for future use.
}

using SchemaCode = uint8_t;

namespace Schema { // Like an enum, but extensible and less strict
  // Allowed values: 0x00 - 0xff
  // All documents have a 1 byte schema field to allow for specification of document schema:

  namespace Generic {
    static const SchemaCode Schemaless = 0x00;
    namespace Primitive {
      static const SchemaCode None    = 0x01;
      static const SchemaCode Boolean = 0x02;
      static const SchemaCode Uint    = 0x03;
      static const SchemaCode Uint8   = 0x04;
      static const SchemaCode Uint16  = 0x05;
      static const SchemaCode Uint32  = 0x06;
      static const SchemaCode Uint64  = 0x07;
      static const SchemaCode Int     = 0x08;
      static const SchemaCode Int8    = 0x09;
      static const SchemaCode Int16   = 0x0a;
      static const SchemaCode Int32   = 0x0b;
      static const SchemaCode Int64   = 0x0c;
      static const SchemaCode Float32 = 0x0d;
      static const SchemaCode Float64 = 0x0e;
      // 0x0e is reserved for future use
    }
    namespace Sequence {
      static const SchemaCode String   = 0x10;
      static const SchemaCode String8  = 0x11;
      static const SchemaCode String16 = 0x12;
      static const SchemaCode String32 = 0x13;
      static const SchemaCode String64 = 0x14;
      static const SchemaCode Binary   = 0x15;
      static const SchemaCode Binary8  = 0x16;
      static const SchemaCode Binary16 = 0x17;
      static const SchemaCode Binary32 = 0x18;
      static const SchemaCode Binary64 = 0x19;
    }
    // 0x1a - 0x1f are reserved for future use
    // 0x2* is available for ad hoc binary dynamically-typed serialization formats defined by programmers/applications
  }
  namespace Framework {
    // 0x30 - 0x4f are reserved for schemas specified by phyllo-specified appliction frameworks
  }
  namespace Application {
    // 0x50 - 0xff is available for ad hoc application schemas defined by programmers/applications, with a recommended allocation convention
    namespace Generic {
      namespace Tuples {
        // 0x5* is recommended to be used for schemas for generic reusable tuples
      }
      namespace Arrays {
        // 0x6* is recommended to be used for schemas for generic reusable arrays
      }
      namespace Maps {
        // 0x7* is recommended to be used for schemas for generic reusable maps
      }
    }
    namespace Debug {
      // 0x8* is recommended to be used for schemas for debugging functionality
    }
    namespace System {
      // 0x9* is recommended to be used for schemas for system management functionality
    }
    namespace Services {
      // 0xa* is recommended to be used for schemas for service management functionality
    }
    namespace Computation {
      // 0xb* is recommended to be used for schemas for intensive computation
    }
    namespace Hardware {
      // 0xc* is recommended to be used for schemas for low-level hardware control
    }
    namespace Devices {
      // 0xd* is recommended to be used for schemas for high-level device control
    }
    namespace Data {
      // 0xe* is recommended to be used for schemas for data management functionality
    }
    namespace Miscellaneous {
      // 0xf* is recommended to be used for schemas for miscellaneous functionality
    }
  }
}

} } }