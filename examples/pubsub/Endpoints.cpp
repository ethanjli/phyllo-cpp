// Test Pub-Sub endpoint handling

// Standard libraries
#include <Arduino.h>

// Third-party libraries

// Phyllo
#include "Phyllo.h"
#include "Phyllo/Tests/Loopback.h"
#include "Endpoints.h"


// COMMUNICATION STACK
// Choose conventional stack configurations

// Serial Port configuration:
auto &SerialStream = Phyllo::IO::USBSerial; // automatically chosen based on platform
// Refer to examples/tests/EchoProtocol.cpp for alternative serial objects to set SerialStream

// Transport Logical Sub-Stack configuration:
using LogicalStack = Phyllo::Protocol::Transport::MinimalLogicalStack;
//using LogicalStack = Phyllo::Protocol::Transport::ReducedLogicalStack;
//using LogicalStack = Phyllo::Protocol::Transport::StandardLogicalStack;

// Application Framework configuration:
namespace Framework = Phyllo::Protocol::Application::PubSub;

// Communication stack automatically created:
using CommunicationStack = Phyllo::SerialCommunicationStack<LogicalStack, Framework::ApplicationStack>;
CommunicationStack communicationStack(SerialStream);


// APPLICATION
// Specify application components

// Instantiate Pub-Sub handlers:
EchoHandler echoHandler(communicationStack.sender);
CopyHandler copyHandler(communicationStack.sender);
ReplyHandler replyHandler(communicationStack.sender);
StringPrefixHandler stringPrefixHandler(communicationStack.sender);
BlinkHandler blinkHandler(communicationStack.sender);
PingPongHandler pingPongHandler(communicationStack.sender);

// Add handlers to the list of handlers to update:
Framework::MsgPackEndpointHandler *pubSubHandlers[] = {
  &echoHandler,
  &copyHandler,
  &replyHandler,
  &stringPrefixHandler,
  &blinkHandler,
  &pingPongHandler
};


// ARDUINO

void setup()
{
  // Debugging setup
  pinMode(LED_BUILTIN, OUTPUT); // Note: this is redundant with BlinkHandler::setup()

  // Serial communication protocol stack: setup
  Phyllo::IO::startSerial(SerialStream);
  communicationStack.setup();

  // Application: setup
  // This is equivalent to calling the setup() method of echoHandler, copyHandler, etc.
  for (auto &handler : pubSubHandlers) handler->setup();
}

void loop() {
  // Event loop updates
  communicationStack.update();
  // This is equivalent to calling the update() method of echoHandler, copyHandler, etc.
  for (auto &handler : pubSubHandlers) handler->update();

  // Serial commmunication protocol stack: receive data
  auto stackReceived = communicationStack.receive();
  if (!stackReceived) return;

  // Application: handle received data
  // This is equivalent to calling the receive(*stackReceived) method of echoHandler, copyHandler, etc.
  for (auto &handler : pubSubHandlers) handler->receive(*stackReceived);
}