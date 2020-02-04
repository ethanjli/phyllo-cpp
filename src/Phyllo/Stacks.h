#pragma once

// Standard libraries
#include <Arduino.h>

// Third-party libraries

// Phyllo
#include "Phyllo/Protocol/Transport/Stacks.h"
#include "Phyllo/Protocol/Stacks.h"
#include "Phyllo/IO/SerialLink.h"

// Standard stacks provide end-to-end communication functionality

namespace Phyllo {

// Standard transport stack configurations for serial communication
using ArduinoMediumStack = Protocol::Transport::StreamMediumStack<Stream>;
using SerialMediumStack = ArduinoMediumStack;

// TODO: make a SerialCommunicationStack class which holds SerialLink, MediumStack, LogicalStack, and TransportStack as members

template<typename LogicalStack, typename ApplicationStack>
class SerialCommunicationStack {
  public:
    using TransportStack = Protocol::Transport::TransportStack<SerialMediumStack, LogicalStack>;
    using ProtocolStack = Protocol::ProtocolStack<TransportStack, ApplicationStack>;

    using TopLink = typename ProtocolStack::TopLink;
    using BottomLink = typename ProtocolStack::BottomLink;

    using ToReceive = typename BottomLink::ToReceive; // The type of data passed up from below
    using Receive = typename TopLink::Receive; // The type of data passed up to above
    using OptionalReceive = typename TopLink::OptionalReceive;
    using Send = typename TopLink::Send; // The type of data passed down from above
    using SendDelegate = typename TopLink::SendDelegate;
    using ToSend = typename BottomLink::ToSend; // The type of data passed down to below
    using ToSendDelegate = typename BottomLink::ToSendDelegate;

    Stream &stream;
    SerialMediumStack medium;
    LogicalStack logical;
    TransportStack transport;
    ApplicationStack application;
    ProtocolStack protocol;

    TopLink &top;
    BottomLink &bottom;
    SendDelegate &sender;

    SerialCommunicationStack(Stream &stream = IO::USBSerial) :
      stream(stream), medium(stream), logical(medium.sender),
      transport(medium, logical), application(transport.sender),
      protocol(transport, application),
      top(protocol.top), bottom(protocol.bottom), sender(protocol.sender) {}

    // Event loop interface

    void setup() {
      protocol.setup();
    }
    void update() {
      protocol.update();
    }

    // Application interface

    OptionalReceive receive() {
      return protocol.receive();
    }

    // No send methods because the calling interface can vary; instead,
    // call protocol.send() or top.send() (they are the same method)!
};

template<typename CommunicationStack, typename ProtocolEventHandler>
class FullStack {
  public:
    using TopLink = typename CommunicationStack::TopLink;
    using BottomLink = typename CommunicationStack::BottomLink;

    using ToReceive = typename BottomLink::ToReceive; // The type of data passed up from below
    using Receive = typename TopLink::Receive; // The type of data passed up to above
    using OptionalReceive = typename TopLink::OptionalReceive;
    using Send = typename TopLink::Send; // The type of data passed down from above
    using SendDelegate = typename TopLink::SendDelegate;
    using ToSend = typename BottomLink::ToSend; // The type of data passed down to below
    using ToSendDelegate = typename BottomLink::ToSendDelegate;

    CommunicationStack &communication;
    ProtocolEventHandler &event;

    TopLink &top;
    BottomLink &bottom;
    SendDelegate &sender;

    FullStack(CommunicationStack &communication, ProtocolEventHandler &event) :
      communication(communication), event(event),
      top(communication.top), bottom(communication.bottom), sender(communication.sender) {
        event.setToSendDelegate(communication.sender);
      }

    // Event loop interface

    void setup() {
      communication.setup();
      event.setup();
    }
    void update() {
      communication.update();
      event.update();
    }

    // Application interface

    OptionalReceive receive() {
      auto communicationReceived = communication.receive();
      if (!communicationReceived) return OptionalReceive();

      event.receive(*communicationReceived);
      return communicationReceived;
    }

    // No send methods because the calling interface can vary; instead,
    // call communication.send() or top.send() (they are the same method)!
};

}