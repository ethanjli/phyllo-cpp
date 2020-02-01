// Test Pub-Sub document routing

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

// Echo and copy handling can be done by using topic endpoints in loop(), without writing an endpoint handler class
Framework::MsgPackTopicEndpoint echoTopicEndpoint("echo", applicationStack.sender);
Framework::MsgPackTopicEndpoint copyTopicEndpoint("copy", applicationStack.sender);

// Reply handling relies on enapsulated data, so it's handled by a single endpoint handler object
class ReplyHandler : public Framework::MsgPackSingleEndpointHandler {
  public:
    ReplyHandler(const ToSendDelegate &delegate) :
      Framework::MsgPackSingleEndpointHandler("reply", delegate) {}

    // Event loop interface

    void setup() {
      replyDocument.header.schema = Phyllo::Protocol::Presentation::Schema::Generic::Sequence::String16; // TODO: this belongs in ReplyNode's setup
      replyDocument.writer.writeAs("hello!");
    }

    // Pub-Sub Node interface

    void endpointReceived(const EndpointDocument &document) {
      send(replyDocument);
    }

  protected:
    EndpointDocument replyDocument;
};

ReplyHandler replyHandler(applicationStack.sender);

// Blink handling relies on internal state, so it's handled by a single endpoint handler object

class BlinkHandler : public Framework::MsgPackSingleEndpointHandler {
  public:
    BlinkHandler(const ToSendDelegate &delegate) :
      Framework::MsgPackSingleEndpointHandler("blink", delegate),
      blinkTimer(100),
      updateTimer(5000) {}

    // Event loop interface

    void setup() {
      pinMode(LED_BUILTIN, OUTPUT);
    }
    void update() {
      if (!blinkTimer.timedOut()) return;

      ledState = !ledState;
      digitalWrite(LED_BUILTIN, ledState);
      blinkTimer.reset();
    }

    // Pub-Sub Node interface

    void endpointReceived(const EndpointDocument &document) {
      if (updateTimer.running()) return;

      document.reader.readAs(blinkTimer.enabled);
      blinkTimer.reset();
      updateTimer.start();

      EndpointDocument blinkDocument;
      blinkDocument.header.schema = Phyllo::Protocol::Presentation::Schema::Generic::Primitive::Boolean;
      blinkDocument.writer.writeAs(blinkTimer.enabled);
      send(blinkDocument);
    }

  protected:
    bool ledState = false;
    Phyllo::Util::TimeoutTimer blinkTimer;
    Phyllo::Util::TimeoutTimer updateTimer;
};

BlinkHandler blinkHandler(applicationStack.sender);

// Ping-pong handling relies on internal state and works on two endpoints, so it's handled by a general endpoint handler object

class PingPongHandler : public Framework::MsgPackEndpointHandler {
  public:
    PingPongHandler(const ToSendDelegate &delegate) :
      pingTopicEndpoint("ping", delegate),
      pongTopicEndpoint("pong", delegate) {}

    // Event loop interface

    void setup() {
      counter = 0;
    }

    // Endpoint handler interface

    void receive(const ToReceive &document) {
      auto pingReceived = pingTopicEndpoint.receive(document);
      if (!pingReceived) return;

      EndpointDocument pongDocument;
      pongDocument.header.schema = Phyllo::Protocol::Presentation::Schema::Generic::Primitive::Uint64;
      pongDocument.writer.writeAs(counter);
      pongTopicEndpoint.send(pongDocument);
      ++counter;
    }

  protected:
    uint64_t counter;
    Framework::MsgPackTopicEndpoint pingTopicEndpoint;
    Framework::MsgPackTopicEndpoint pongTopicEndpoint;
};

PingPongHandler pingPongHandler(applicationStack.sender);


Framework::MsgPackEndpointHandler *pubSubHandlers[] = {
  &replyHandler,
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
  auto echoReceived = echoTopicEndpoint.receive(*stackReceived);
  if (echoReceived) echoTopicEndpoint.send(*echoReceived);

  auto copyReceived = copyTopicEndpoint.receive(*stackReceived);
  if (copyReceived) copyTopicEndpoint.send(Phyllo::Tests::copyFlatDocument(*copyReceived));

  for (auto &handler : pubSubHandlers) handler->receive(*stackReceived);
}