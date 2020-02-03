// Test Pub-Sub endpoint handling

// Standard libraries
#include <Arduino.h>


// Third-party libraries

// Phyllo
#include "Phyllo.h"
#include "Phyllo/Tests/Loopback.h"
#include "Endpoints.h"

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

// Application Stack configuration:
using ApplicationStack = Phyllo::Protocol::Application::PubSubStack;

// Stack types automatically deduced

using TransportStack = Phyllo::Protocol::Transport::TransportStack<MediumStack, LogicalStack>;
using ProtocolStack = Phyllo::Protocol::ProtocolStack<TransportStack, ApplicationStack>;

// Stack classes automatically created

// TODO: use the configurations to make a templated complete stack which owns and initializes all of the following:
MediumStack mediumStack(SerialStream);
LogicalStack logicalStack(mediumStack.sender);
TransportStack transportStack(mediumStack, logicalStack);
ApplicationStack applicationStack(transportStack.sender);
ProtocolStack protocolStack(transportStack, applicationStack);

// Application

namespace Framework = Phyllo::Protocol::Application::PubSub;

EchoHandler echoHandler(applicationStack.sender);
CopyHandler copyHandler(applicationStack.sender);
ReplyHandler replyHandler(applicationStack.sender);
StringPrefixHandler stringPrefixHandler(applicationStack.sender);
BlinkHandler blinkHandler(applicationStack.sender);
PingPongHandler pingPongHandler(applicationStack.sender);

Framework::MsgPackEndpointHandler *pubSubHandlers[] = {
  &echoHandler,
  &copyHandler,
  &replyHandler,
  &stringPrefixHandler,
  &blinkHandler,
  &pingPongHandler
};

// Arduino

void setup()
{
  // Debugging setup
  pinMode(LED_BUILTIN, OUTPUT);

  // Serial communication protocol stack: setup
  Phyllo::IO::startSerial(SerialStream, kUSBSerialRate);
  protocolStack.setup();

  // Application: setup
  for (auto &handler : pubSubHandlers) handler->setup();
}

void loop() {
  // Event loop updates
  protocolStack.update();
  for (auto &handler : pubSubHandlers) handler->update();

  // Serial commmunication protocol stack: receive data
  auto stackReceived = protocolStack.receive();
  if (!stackReceived) return;

  // Application: handle received data

  for (auto &handler : pubSubHandlers) handler->receive(*stackReceived);
}