#pragma once

// Standard libraries

// Third-party libaries
#include <etl/string_view.h>
#include <etl/functional.h>

// Phyllo
#include "Phyllo/Types.h"
#include "Phyllo/Protocol/Application/PubSub/DocumentLink.h"

// Routing allows different objects to handle different documents depending on their respective topics.

namespace Phyllo { namespace Protocol { namespace Application { namespace PubSub {

// TopicFilter allows filtering documents by topic.
// Topic filtering can be done by an exact match on the provided topic or a prefix match on the provided topic

class TopicFilter {
  public:
    const ByteBufferView filter;

    TopicFilter(const char *filter) :
      TopicFilter(etl::string_view(filter)) {}
    template<typename ByteArray>
    TopicFilter(const ByteArray &filter) :
      filter(
        reinterpret_cast<const uint8_t *>(filter.begin()),
        reinterpret_cast<const uint8_t *>(filter.end())
      ) {}
    TopicFilter(const ByteBufferView &filter) : filter(filter) {}
    TopicFilter(const TopicFilter &filter) = delete; // prevent accidental copy-by-value

    // TopicFilter interface

    bool matches(const ByteBufferView &topic) const {
      return topic == filter;
    }

    bool prefixes(const ByteBufferView &topic) const {
      return (
        (topic.size() >= filter.size())
        && (ByteBufferView(topic.data(), topic.data() + filter.size()) == filter)
      );
    }

    ByteBufferView suffix(const ByteBufferView &topic) const { // Warning: does not check whether the filter matches the prefix!
      if (filter.size() >= topic.size()) return ByteBufferView();

      return ByteBufferView(topic.begin() + filter.size(), topic.end());
    }
};

// TopicEndpoint allows receiving and sending documents on one topic with an exact match

template<Presentation::SerializationFormatCode Format>
class TopicEndpoint {
  public:
    using Topic = ByteBufferView;
    using ToReceive = Document<Format>; // The type of data passed up from below
    using Receive = Presentation::Document<Format>; // The type of data passed up to above
    using OptionalReceive = Util::Optional<Receive>;
    using Send = Presentation::Document<Format>;
    using SendDelegate = etl::delegate<bool(const Send &)>;
    using ToSend = Document<Format>; // The type of data passed down to below
    using ToSendDelegate = etl::delegate<bool(const ToSend &)>;

    const TopicFilter filter;

    template<typename Filter>
    TopicEndpoint(const Filter &filter, const ToSendDelegate &delegate) :
      filter(filter), sender(delegate) {}
    TopicEndpoint(const TopicEndpoint &node) = delete;

    OptionalReceive receive(const ToReceive &document) {
      OptionalReceive received;
      if (!filter.matches(document.topic())) return received;
      return document;
    }

    bool send(const Send &document) {
      Document<Format> sendDocument;
      sendDocument.setTopic(filter.filter);
      sendDocument.read(document.buffer());
      return sender(sendDocument);
    }

  protected:
    const ToSendDelegate &sender;
};

// EndpointHandler is a base class for handling received pub-sub documents

template<Presentation::SerializationFormatCode Format>
class EndpointHandler {
  public:
    using ToReceive = typename TopicEndpoint<Format>::ToReceive; // The type of data passed up from below
    using ToSendDelegate = typename TopicEndpoint<Format>::ToSendDelegate;
    using EndpointDocument = typename TopicEndpoint<Format>::Receive;

    // Event loop interface
    virtual void setup() {}
    virtual void update() {}

    // Endpoint handler interface
    virtual void receive(const ToReceive &document) {}
};

template<Presentation::SerializationFormatCode Format>
using EndpointHandlerReference = etl::reference_wrapper<EndpointHandler<Format>>;

// SingleEndpointHandler is a unit of the application which handles received documents on a single TopicEndpoint

template<Presentation::SerializationFormatCode Format>
class SingleEndpointHandler : public EndpointHandler<Format> {
  public:
    using ToReceive = typename EndpointHandler<Format>::ToReceive; // The type of data passed up from below
    using ToSendDelegate = typename EndpointHandler<Format>::ToSendDelegate;
    using EndpointDocument = typename EndpointHandler<Format>::EndpointDocument;

    template<typename Filter>
    SingleEndpointHandler(const Filter &filter, const ToSendDelegate &delegate) :
      topicEndpoint(filter, delegate) {}
    SingleEndpointHandler(const SingleEndpointHandler &handler) = delete;

    // Endpoint handler interface

    void receive(const ToReceive &document) {
      auto topicReceived = topicEndpoint.receive(document);
      if (!topicReceived) return;

      endpointReceived(*topicReceived);
    }

    // Single-endpoint handler interface

    virtual void endpointReceived(const EndpointDocument &document) {}

    bool send(const EndpointDocument &document) {
      return topicEndpoint.send(document);
    }

  protected:
    TopicEndpoint<Format> topicEndpoint;
};

} } } }