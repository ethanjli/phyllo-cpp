#pragma once

// Standard libraries

// Third-party libraries
#include <etl/delegate.h>

// Phyllo
#include "Phyllo/Protocol/Types.h"
#include "Phyllo/Util/Optional.h"
#include "Phyllo/Protocol/Presentation/DocumentLink.h"
#include "Phyllo/Protocol/Presentation/MessagePack.h"
#include "PubSub/MessageLink.h"
#include "PubSub/DocumentLink.h"
#include "PubSub/Endpoint.h"

// Stacks orchestrate the flow of data through protocol layers

namespace Phyllo { namespace Protocol {

namespace Presentation { namespace MsgPack {
  using Document = Presentation::Document<kFormat>;
  using DocumentLink = Presentation::DocumentLink<kFormat>;
} }

namespace Application { namespace PubSub {
  using MsgPackDocumentLink = DocumentLink<Presentation::MsgPack::kFormat>;
  using MsgPackEndpoint = Endpoint<Presentation::MsgPack::kFormat>;
  using MsgPackEndpointHandler = EndpointHandler<Presentation::MsgPack::kFormat>;
  using MsgPackSingleEndpointHandler = SingleEndpointHandler<Presentation::MsgPack::kFormat>;
} }

} }

namespace Phyllo { namespace Protocol { namespace Application {

class MinimalStack {
  public:
    using TopLink = Presentation::MsgPack::DocumentLink;
    using BottomLink = Presentation::MsgPack::DocumentLink;

    using ToReceive = BottomLink::ToReceive; // The type of data passed up from below
    using Receive = TopLink::Receive; // The type of data passed up to above
    using OptionalReceive = TopLink::OptionalReceive;
    using Send = TopLink::Send; // The type of data passed down from above
    using SendDelegate = TopLink::SendDelegate;
    using ToSend = BottomLink::ToSend; // The type of data passed down to below
    using ToSendDelegate = BottomLink::ToSendDelegate;

    Presentation::MsgPack::DocumentLink document;

    TopLink &top;
    BottomLink &bottom;
    SendDelegate sender;

    MinimalStack(const ToSendDelegate &toSender) :
      document(toSender),
      top(document), bottom(document),
      sender(SendDelegate::create<TopLink, &TopLink::send>(top)) {}

    // Event loop interface

    void setup() {
      document.setup();
    }
    void update() {
      document.update();
    }

    // DocumentLink interface

    OptionalReceive receive(const ByteBufferView &buffer) {
      return document.receive(buffer);
    }

    bool send(const Send &document) { return top.send(document); }
};

class PubSubStack {
  public:
    using TopLink = PubSub::MsgPackDocumentLink;
    using BottomLink = PubSub::MessageLink;

    using ToReceive = BottomLink::ToReceive; // The type of data passed up from below
    using Receive = TopLink::Receive; // The type of data passed up to above
    using OptionalReceive = TopLink::OptionalReceive;
    using Send = TopLink::Send; // The type of data passed down from above
    using SendDelegate = TopLink::SendDelegate;
    using ToSend = BottomLink::ToSend; // The type of data passed down to below
    using ToSendDelegate = BottomLink::ToSendDelegate;

    PubSub::MessageLink message;
    PubSub::MsgPackDocumentLink document;

    TopLink &top;
    BottomLink &bottom;
    SendDelegate sender;

    PubSubStack(const ToSendDelegate &toSender) :
      message(toSender), document(intermediateToSender),
      top(document), bottom(message),
      sender(SendDelegate::create<TopLink, &TopLink::send>(top)) {;
        intermediateToSender = IntermediateToSendDelegate::create<
          PubSubStack, &PubSubStack::toSend
        >(*this);
      }

    // Event loop interface

    void setup() {
      message.setup();
      document.setup();
    }
    void update() {
      message.update();
      document.update();
    }

    // Named-topic documentLink interface

    OptionalReceive receive(const ByteBufferView &buffer) {
      auto messageReceived = message.receive(buffer);
      if (!messageReceived) return OptionalReceive();

      return document.receive(messageReceived->topic(), messageReceived->payload());
    }

    bool send(const Send &document) {
      return top.send(document);
    }

  protected:
    using IntermediateToSendDelegate = PubSub::MsgPackDocumentLink::ToSendDelegate;
    IntermediateToSendDelegate intermediateToSender;

    bool toSend(
      const ByteBufferView &topic, const PubSub::MsgPackDocumentLink::ToSend &body,
      DataUnitTypeCode type
    ) {
      return message.send(topic, body, type);
    }
};

} } }