// Test whether the protocol stack correctly echoes received data

// Standard libraries
#include <Arduino.h>

// Third-party libraries

// Phyllo
#include "Phyllo.h"
#include "Phyllo/Tests/Loopback.h"


// COMMUNICATION STACK
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

// Transport Logical Sub-Stack configuration:
using LogicalStack = Phyllo::Protocol::Transport::MinimalLogicalStack;
//using LogicalStack = Phyllo::Protocol::Transport::ReducedLogicalStack;
//using LogicalStack = Phyllo::Protocol::Transport::StandardLogicalStack;

// Application Stack configuration:
//using ApplicationStack = Phyllo::Protocol::Application::MinimalStack;
using ApplicationStack = Phyllo::Protocol::Application::PubSubStack;

// Communication stack automatically created:
using CommunicationStack = Phyllo::SerialCommunicationStack<LogicalStack, ApplicationStack>;
CommunicationStack communicationStack(SerialStream);


// ARDUINO

void setup()
{
  // Debugging setup
  pinMode(LED_BUILTIN, OUTPUT);

  // Serial communication protocol stack: setup
  Phyllo::IO::startSerial(SerialStream, kUSBSerialRate);
  communicationStack.setup();
}

void loop() {
  // Event loop updates
  communicationStack.update();

  Phyllo::Tests::loopAnnounce(communicationStack.top);
}