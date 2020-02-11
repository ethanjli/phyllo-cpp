// Test whether the transport stack correctly echoes received data

// Standard libraries
#include <Arduino.h>

// Third-party libraries

// Phyllo
#include "Phyllo.h"
#include "Phyllo/Tests/Loopback.h"


// TRANSPORT STACK
// Choose conventional stack configurations

// Serial Port configuration:
auto &SerialStream = Phyllo::IO::USBSerial; // automatically chosen based on platform
//auto &SerialStream = Serial; // Default USB on Arduino & Teensy boards
//auto &SerialStream = SerialUSB; // Native USB on Due & Zero boards
//auto &SerialStream = Serial1; // Hardware Serial 1
//auto &SerialStream = Serial2; // Hardware Serial 2
//auto &SerialStream = Serial3; // Hardware Serial 3
//auto &SerialStream = Serial4; // Hardware Serial 4
//auto &SerialStream = Serial5; // Hardware Serial 5
//auto &SerialStream = Serial6; // Hardware Serial 6

// Serial Port Data Rate configuration (ignored for Due on Native USB port, Micro, Leonardo, and Teensy):
static const long kUSBSerialRate = Phyllo::IO::kUSBSerialRate; // automatically chosen by build flag, defaults to 115200

Phyllo::Protocol::Transport::StreamLink<Stream> streamLink(SerialStream);

// ARDUINO

void setup()
{
  // Debugging setup
  pinMode(LED_BUILTIN, OUTPUT);

  // Serial communication transport protocol stack: setup
  Phyllo::IO::startSerial(SerialStream, kUSBSerialRate);
  streamLink.setup();
}

void loop() {
  streamLink.update();
  if (streamLink.hasRead()) streamLink.send(streamLink.read());
}