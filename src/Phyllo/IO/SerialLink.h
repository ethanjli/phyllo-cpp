#pragma once

// Standard libraries
#include <Arduino.h>

// Third-party libraries

// Phyllo
#include "Phyllo/Platform.h"
#include "ArduinoStreamLink.h"

// Serial Link handles serial I/O over UART or USB

#ifndef PHYLLO_USB_SERIAL_RATE
#define PHYLLO_USB_SERIAL_RATE 115200
#endif

#if PHYLLO_PLATFORM == PHYLLO_PLATFORM_ATMELAVR
#define PHYLLO_USB_SERIAL PHYLLO_USB_SERIAL_DEFAULT
#elif PHYLLO_PLATFORM == PHYLLO_PLATFORM_ATMELAVR
#define PHYLLO_USB_SERIAL PHYLLO_USB_SERIAL_DEFAULT
#endif

#define PHYLLO_USB_SERIAL_DEFAULT 0
#define PHYLLO_USB_SERIAL_DUE 1
#ifndef PHYLLO_USB_SERIAL
#define PHYLLO_USB_SERIAL PHYLLO_SERIAL_DEFAULT
#endif


namespace Phyllo { namespace IO {

static const long kUSBSerialRate = PHYLLO_USB_SERIAL_RATE; // This has no effect on Serial over native USB ports (as opposed to FTDI)

template<typename SerialClass>
void startSerial(SerialClass &serial, long serialDataRate = kUSBSerialRate, bool waitSerial = false) {
  serial.begin(serialDataRate);
  if (!waitSerial) return;

  while (!serial) ;
}

// Native USB

#if PHYLLO_USB_SERIAL == PHYLLO_USB_SERIAL_DEFAULT
// UART USB (e.g. Uno and Due with programming port)
auto &USBSerial = Serial;
#elif PHYLLO_USB_SERIAL == PHYLLO_USB_SERIAL_DUE
// Native USB (only on Due or Zero with native port)
auto &USBSerial = SerialUSB;
#endif

} }