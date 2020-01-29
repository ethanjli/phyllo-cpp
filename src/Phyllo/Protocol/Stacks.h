#pragma once

// Standard libraries

// Third-party libraries

// Phyllo
#include "Phyllo/Protocol/Transport/Stacks.h"
#include "Phyllo/Protocol/Application/Stacks.h"

// Stacks orchestrate the flow of data through protocol layers

namespace Phyllo { namespace Protocol {

template<typename TransportStack, typename ApplicationStack>
class ProtocolStack {
  public:
    using TopLink = typename ApplicationStack::TopLink;
    using BottomLink = typename TransportStack::BottomLink;

    using ToReceive = typename BottomLink::ToReceive; // The type of data passed up from below
    using Receive = typename TopLink::Receive; // The type of data passed up to above
    using OptionalReceive = typename TopLink::OptionalReceive;
    using Send = typename TopLink::Send; // The type of data passed down from above
    using SendDelegate = typename TopLink::SendDelegate;
    using ToSend = typename BottomLink::ToSend; // The type of data passed down to below
    using ToSendDelegate = typename BottomLink::ToSendDelegate;

    TransportStack &transport;
    ApplicationStack &application;

    TopLink &top;
    BottomLink &bottom;
    SendDelegate &sender;

    ProtocolStack(TransportStack &transport, ApplicationStack &application) :
      transport(transport), application(application),
      top(application.top), bottom(transport.bottom), sender(application.sender) {}

    // Event loop interface

    void setup() {
      transport.setup();
      application.setup();
    }
    void update() {
      transport.update();
      application.update();
    }

    // Application interface
    
    OptionalReceive receive() {
      auto transportReceived = transport.receive();
      if (!transportReceived) return OptionalReceive();

      return application.receive(getPayload(*transportReceived));
    }

    // No send methods because the calling interface can vary; instead, call top.send()!
};

} }