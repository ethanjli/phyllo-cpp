#pragma once

#undef min
#undef max

// Platform-specific flags

#define PHYLLO_PLATFORM_UNKNOWN 1
#define PHYLLO_PLATFORM_ATMELAVR 1
#define PHYLLO_PLATFORM_ATMELSAM 2
#define PHYLLO_PLATFORM_TEENSY 3

#ifndef PHYLLO_PLATFORM

#if defined (__AVR__) || (__avr__)
#define PHYLLO_PLATFORM PHYLLO_PLATFORM_ATMELAVR
#elif defined (CORE_TEENSY)
#define PHYLLO_PLATFORM PHYLLO_PLATFORM_TEENSY
#elif defined (__arm__)
#define PHYLLO_PLATFORM PHYLLO_PLATFORM_ATMELSAM
#pragma warn("Assuming platform is ATMELSAM!")
#else
#define PHYLLO_PLATFORM PHYLLO_PLATFORM_UNKNOWN
#pragma warn("Unknown platform!")
#endif

#endif

#if PHYLLO_PLATFORM == PHYLLO_PLATFORM_ATMELAVR
#define ETL_NO_STL
#ifndef PHYLLO_TRANSPORT_CHUNK_SIZE_LIMIT
#define PHYLLO_TRANSPORT_CHUNK_SIZE_LIMIT 127
#pragma warn("Serial frames are limited to 127 bytes instead of 255 bytes!")
#endif
// Polyfills for missing features needed by ETL on AVR 8-bit microcontrollers
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

#define PHYLLO_DEBUG_LED digitalWrite(LED_BUILTIN, HIGH);