#pragma once

#undef min
#undef max

// Platform-specific flags

#define PHYLLO_PLATFORM_UNKNOWN 0
#define PHYLLO_PLATFORM_ATMELAVR 1
#define PHYLLO_PLATFORM_ATMELSAM 2
#define PHYLLO_PLATFORM_TEENSY 3

// Deduce platform from Arduino framework
#ifndef PHYLLO_PLATFORM
#if defined (ARDUINO_ARCH_AVR)
#define PHYLLO_PLATFORM PHYLLO_PLATFORM_ATMELAVR
#elif defined (ARDUINO_ARCH_MEGAAVR)
#define PHYLLO_PLATFORM PHYLLO_PLATFORM_ATMELAVR
#define PHYLLO_PLATFORM_ATMELMEGAAVR
#elif defined (ARDUINO_ARCH_SAM)
#define PHYLLO_PLATFORM PHYLLO_PLATFORM_ATMELSAM
#elif defined (CORE_TEENSY)
#define PHYLLO_PLATFORM PHYLLO_PLATFORM_TEENSY
#endif
#endif

// Attempt to automatically deduce platform if defines from Arduino framework are missing
#ifndef PHYLLO_PLATFORM
#if defined (__AVR__) || (__avr__)
#define PHYLLO_PLATFORM PHYLLO_PLATFORM_ATMELAVR
#pragma message("Assuming platform is ATMELAVR!")
#elif defined (__arm__)
#define PHYLLO_PLATFORM PHYLLO_PLATFORM_ATMELSAM
#pragma message("Assuming platform is ATMELSAM!")
#else
#define PHYLLO_PLATFORM PHYLLO_PLATFORM_UNKNOWN
#pragma message("Unknown platform!")
#endif
#endif

// Set default values for platforms
#if PHYLLO_PLATFORM == PHYLLO_PLATFORM_ATMELAVR
#define ETL_NO_STL
#ifndef PHYLLO_TRANSPORT_CHUNK_SIZE_LIMIT
#ifdef PHYLLO_PLATFORM_ATMELMEGAAVR
#define PHYLLO_TRANSPORT_CHUNK_SIZE_LIMIT 255
#else
#define PHYLLO_TRANSPORT_CHUNK_SIZE_LIMIT 127
#pragma message("Serial frames are limited to 127 bytes instead of 255 bytes!")
#endif
#endif
// Polyfills for missing features needed by ETL on AVR 8-bit microcontrollers which aren't provided by AVR or ArduinoSTL
namespace std {
  template <typename T>
  T* addressof(T& t) {
    return reinterpret_cast<T*>(&const_cast<char&>(reinterpret_cast<const volatile char&>(t)));
  }
}
#define HUGE_VALF __builtin_huge_valf()
#define HUGE_VAL __builtin_huge_val()
#define HUGE_VALL __builtin_huge_vall()
#define nan __builtin_nan
#define nanf __builtin_nanf
#define nanl __builtin_nanl

#elif PHYLLO_PLATFORM == PHYLLO_PLATFORM_ATMELSAM
#define ETL_NO_STL
#endif

// This forward declaration is needed for ETL v16.4.1's utilities.h to find swap correctly, but should be fixed in v16.4.3.
namespace etl {
  template <typename T>
  void swap(T& a, T& b);
}

// Define macros for easier debugging
#define PHYLLO_DEBUG_LED digitalWrite(LED_BUILTIN, HIGH);