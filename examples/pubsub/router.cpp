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
// They are implemented here without using an endpoint handler class, just to show how it's don
Framework::MsgPackEndpoint echoEndpoint("echo", applicationStack.sender);
Framework::MsgPackEndpoint copyEndpoint("copy", applicationStack.sender);

// Reply handling relies on enapsulated data, so it's handled by a single endpoint handler object
// It is implmented as a basic example of how to write a basic single endpoint handler object
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

// String prefix handling works on a single endpoint, so it's handled by a single endpoint handler object
// It is implemented as an example of how to write a handler which receives and sends basic structs
class StringPrefixHandler : public Framework::MsgPackSingleEndpointHandler {
  public:
    StringPrefixHandler(const ToSendDelegate &delegate) :
      Framework::MsgPackSingleEndpointHandler("prefix", delegate) {}

    // Pub-Sub Node interface

    void endpointReceived(const EndpointDocument &document) {
      Request request;
      if (!document.reader.readClassAs(request)) return;

      EndpointDocument responseDocument;
      Response response(request);
      response.responseString.append(request.prefixString.data(), request.prefixString.size());
      response.responseString.append(request.rootString.data(), request.rootString.size());
      if (!responseDocument.writer.writeClassAs(response)) return;
      send(responseDocument);
    }

  protected:
    struct Request {
      static const Phyllo::Protocol::Presentation::SerializationFormatCode kFormat = 0x80;
      using MessageReader = ToReceive::Reader;
      using MessageWriter = ToReceive::Writer;

      Phyllo::StringView rootString;
      Phyllo::StringView prefixString;

      // Serializable Document interface

      bool read(MessageReader &reader) { // used by DocumentReader to deserialize from a MessagePack Document body
        reader.startArrayLength(2);
        rootString = reader.readStringView();
        prefixString = reader.readStringView();
        reader.finishArray();

        return !reader.error();
      }

      bool write(MessageWriter &writer) const { // used by DocumentWriter to serialize into a MessagePack Document body
        writer.startArray(2);
        writer.writeString(rootString);
        writer.writeString(prefixString);
        writer.finishArray();
        return !writer.error();
      }
    };

    struct Response {
      static const Phyllo::Protocol::Presentation::SerializationFormatCode kFormat = 0x81;
      using MessageWriter = ToReceive::Writer;

      Response(const Request &request) :
        request(request) {}

      const Request &request;
      Phyllo::String64 responseString;

      // Serializable Document interface

      bool write(MessageWriter &writer) const { // used by DocumentWriter to serialize into a MessagePack Document body
        writer.startArray(2);
        writer.writeClass(request);
        writer.writeString(responseString);
        writer.finishArray();
        return !writer.error();
      }
    };
};

StringPrefixHandler stringPrefixHandler(applicationStack.sender);

// Blink handling relies on internal state, so it's handled by a single endpoint handler object
// It is implemented as an example of how to write a stateful endpoint handler object which
// also has its own event-loop behavior
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
// It is implemented as an example of how to write a general endpoint handler object with multiple endpoints
class PingPongHandler : public Framework::MsgPackEndpointHandler {
  public:
    PingPongHandler(const ToSendDelegate &delegate) :
      pingEndpoint("ping", delegate),
      pongEndpoint("pong", delegate) {}

    // Event loop interface

    void setup() {
      counter = 0;
    }

    // Endpoint handler interface

    void receive(const ToReceive &document) {
      auto pingReceived = pingEndpoint.receive(document);
      if (!pingReceived) return;

      EndpointDocument pongDocument;
      pongDocument.header.schema = Phyllo::Protocol::Presentation::Schema::Generic::Primitive::Uint64;
      pongDocument.writer.writeAs(counter);
      pongEndpoint.send(pongDocument);
      ++counter;
    }

  protected:
    uint64_t counter;
    Framework::MsgPackEndpoint pingEndpoint;
    Framework::MsgPackEndpoint pongEndpoint;
};

PingPongHandler pingPongHandler(applicationStack.sender);


Framework::MsgPackEndpointHandler *pubSubHandlers[] = {
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
  auto echoReceived = echoEndpoint.receive(*stackReceived);
  if (echoReceived) echoEndpoint.send(*echoReceived);

  auto copyReceived = copyEndpoint.receive(*stackReceived);
  if (copyReceived) copyEndpoint.send(Phyllo::Tests::copyFlatDocument(*copyReceived));

  for (auto &handler : pubSubHandlers) handler->receive(*stackReceived);
}