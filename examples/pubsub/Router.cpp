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
EchoHandler echoHandler;
CopyHandler copyHandler;
ReplyHandler replyHandler;
StringPrefixHandler stringPrefixHandler;
BlinkHandler blinkHandler;
PingPongHandler pingPongHandler;

// Add handlers to the router:
using Router = Framework::MsgPackRouter<>; // Give router the capacity to hold up to 256 handlers (default capacity)
//using Router = Framework::MsgPackRouter<512>; // Give router the capacity to hold up to 512 handlers, or any arbitrary number you specify
Router router(
  echoHandler,
  copyHandler,
  replyHandler,
  stringPrefixHandler,
  blinkHandler,
  pingPongHandler
);

// Full stack automatically created:
using FullStack = Phyllo::FullStack<CommunicationStack, Router>;
FullStack fullStack(communicationStack, router);


// ARDUINO

void setup()
{
  // Debugging setup
  pinMode(LED_BUILTIN, OUTPUT); // Note: this is redundant with BlinkHandler::setup()

  // Serial communication & application protocol stack: setup
  Phyllo::IO::startSerial(SerialStream); // TODO: move this into communication stack if possible
  fullStack.setup();
}

void loop() {
  fullStack.update();
  fullStack.receive();
}