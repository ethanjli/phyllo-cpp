#pragma once

// Sample pub-sub endpoints for testing

// Third-party libraries

// Phyllo
#include "Phyllo.h"
#include "Phyllo/Util/Timing.h"
#include "Phyllo/Tests/Loopback.h"

namespace Framework = Phyllo::Protocol::Application::PubSub;

// Echo handling is handled by a single endpoint handler object
// It is implmented as a basic example of how to write a basic single endpoint handler object
class EchoHandler : public Framework::MsgPackSingleEndpointHandler {
  public:
    EchoHandler() :
      Framework::MsgPackSingleEndpointHandler("echo") {}
    EchoHandler(const ToSendDelegate &delegate) :
      Framework::MsgPackSingleEndpointHandler("echo", delegate) {}

    // Pub-Sub Node interface

    void endpointReceived(const EndpointDocument &document) {
      send(document); // this just sends the document directly
    }
};

// Copy handling is handled by a single endpoint handler object
// It is implmented as a basic example of how to write a basic single endpoint handler object
class CopyHandler : public Framework::MsgPackSingleEndpointHandler {
  public:
    CopyHandler() :
      Framework::MsgPackSingleEndpointHandler("copy") {}
    CopyHandler(const ToSendDelegate &delegate) :
      Framework::MsgPackSingleEndpointHandler("copy", delegate) {}

    // Pub-Sub Node interface

    void endpointReceived(const EndpointDocument &document) {
      send(Phyllo::Tests::copyFlatDocument(document)); // this copies the document into a new document in memory and sends it
    }
};

// Reply handling relies on enapsulated data, so it's handled by a single endpoint handler object
// It is implmented as a basic example of how to write a basic single endpoint handler object
class ReplyHandler : public Framework::MsgPackSingleEndpointHandler {
  public:
    ReplyHandler() :
      Framework::MsgPackSingleEndpointHandler("reply") {}
    ReplyHandler(const ToSendDelegate &delegate) :
      Framework::MsgPackSingleEndpointHandler("reply", delegate) {}

    // Event loop interface

    void setup() {
      replyDocument.header.schema = Phyllo::Protocol::Presentation::Schema::Generic::Sequence::String16; // TODO: this belongs in ReplyNode's setup
      replyDocument.writer.writeAs("hello!"); // the reply document is just the string "hello!"
    }

    // Pub-Sub Node interface

    void endpointReceived(const EndpointDocument &document) {
      send(replyDocument); // this just sends the reply document initialized in setup()
    }

  protected:
    EndpointDocument replyDocument;
};

// String prefix handling works on a single endpoint, so it's handled by a single endpoint handler object
// It is implemented as an example of how to write a handler which receives and sends basic structs
class StringPrefixHandler : public Framework::MsgPackSingleEndpointHandler {
  public:
    StringPrefixHandler() :
      Framework::MsgPackSingleEndpointHandler("prefix") {}
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

// Blink handling relies on internal state, so it's handled by a single endpoint handler object
// It is implemented as an example of how to write a stateful endpoint handler object which
// also has its own event-loop behavior
class BlinkHandler : public Framework::MsgPackSingleEndpointHandler {
  public:
    BlinkHandler() :
      Framework::MsgPackSingleEndpointHandler("blink"),
      blinkTimer(100),
      updateTimer(0) {}
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
      digitalWrite(LED_BUILTIN, ledState ? HIGH : LOW); // ?: operator needed for compatibiity with Arduino megaAVR core API; see https://github.com/arduino/ArduinoCore-API/issues/25
      blinkTimer.reset();
    }

    // Pub-Sub Node interface

    void endpointReceived(const EndpointDocument &document) {
      if (updateTimer.running()) return; // the update timer prevents the blink state from being updated more than once every 5 sec

      document.reader.readAs(blinkTimer.enabled); // this assumes the document is just a bool for whether to blink
      blinkTimer.reset(); // start the timer for blinking
      updateTimer.start(); // start the update timer

      EndpointDocument blinkDocument;
      blinkDocument.header.schema = Phyllo::Protocol::Presentation::Schema::Generic::Primitive::Boolean;
      blinkDocument.writer.writeAs(blinkTimer.enabled); // this sends a document which is just a bool for whether the led is blinking
      send(blinkDocument);
    }

  protected:
    bool ledState = false;
    Phyllo::Util::TimeoutTimer blinkTimer;
    Phyllo::Util::TimeoutTimer updateTimer;
};

// Ping-pong handling relies on internal state and works on two endpoints, so it's handled by a general endpoint handler object
// It is implemented as an example of how to write a general endpoint handler object with multiple endpoints
class PingPongHandler : public Framework::MsgPackEndpointHandler {
  public:
    PingPongHandler() :
      pingEndpoint("ping"),
      pongEndpoint("pong") {}
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
      pongDocument.writer.writeAs(counter); // this sends a document which is just a counter of the number of pings received
      pongEndpoint.send(pongDocument);
      ++counter;
    }

    void setToSendDelegate(const ToSendDelegate &delegate) {
      pingEndpoint.setToSendDelegate(delegate);
      pongEndpoint.setToSendDelegate(delegate);
    }

  protected:
    uint64_t counter;
    Framework::MsgPackEndpoint pingEndpoint;
    Framework::MsgPackEndpoint pongEndpoint;
};