// Test whether the transport stack correctly echoes received data

// Standard libraries
#include <Arduino.h>

// Third-party libraries

// Phyllo
#include "Phyllo.h"
#include "Phyllo/Tests/Loopback.h"

// Choose conventional stack configurations

// Serial Port configuration:
auto &SerialStream = Phyllo::IO::USBSerial; // automatically chosen based on platform
//auto &SerialStream = Serial; // Default USB on Arduino & Teensy boards
//auto &SerialStream = SerialUSB; // Native USB on Due & Zero boards
//auto &SerialStream = Serial1; // Hardware Serial
//auto &SerialStream = Serial2; // Hardware Serial
//auto &SerialStream = Serial3; // Hardware Serial
//auto &SerialStream = Serial4; // Hardware Serial
//auto &SerialStream = Serial5; // Hardware Serial
//auto &SerialStream = Serial6; // Hardware Serial

// Serial Port Data Rate configuration (ignored for Due on Native USB port, Micro, Leonardo, and Teensy):
static const long kUSBSerialRate = Phyllo::IO::kUSBSerialRate; // automatically chosen by build flag, defaults to 115200

// I/O + Transport Medium Sub-Stack configuration:
using MediumStack = Phyllo::SerialMediumStack;

// Transport Logical Sub-Stack configuration:
using LogicalStack = Phyllo::Protocol::Transport::MinimalLogicalStack;
//using LogicalStack = Phyllo::Protocol::Transport::ReducedLogicalStack;
//using LogicalStack = Phyllo::Protocol::Transport::StandardLogicalStack;


// Stack types automatically deduced

using TransportStack = Phyllo::Protocol::Transport::TransportStack<MediumStack, LogicalStack>;

// Stack classes automatically created

// TODO: use the configurations to make a templated complete stack which owns and initializes all of the following:
MediumStack mediumStack(SerialStream);
LogicalStack logicalStack(mediumStack.sender);
TransportStack transportStack(mediumStack, logicalStack);

// Arduino

void setup()
{
  Phyllo::IO::startSerial(SerialStream, kUSBSerialRate);
  pinMode(LED_BUILTIN, OUTPUT);
  transportStack.setup();
}

void loop() {
  // Transport stack tests
  //Phyllo::Tests::loopAnnounce(transportStack);
  //Phyllo::Tests::loopReply(transportStack);
  Phyllo::Tests::loopEchoPayload(transportStack);
}