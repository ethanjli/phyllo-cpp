#pragma once

// Standard libraries

// Third-party libaries
#include <etl/functional.h>

// Phyllo
#include "Phyllo/Types.h"
#include "Phyllo/Protocol/Presentation/Types.h"

// Endpoints allow different objects to handle different documents depending on their respective named endpoints.

namespace Phyllo { namespace Protocol { namespace Application {

// NameFilter allows filtering documents by topic.
// Topic filtering can be done by an exact match on the provided topic or a prefix match on the provided topic

class NameFilter {
  public:
    const ByteBufferView filter;
    uint8_t byteFilter;

    NameFilter(uint8_t singleByteFilter) :
      filter(ByteBufferView(&singleByteFilter, 1)), byteFilter(singleByteFilter) {}
    NameFilter(const char *filter) :
      NameFilter(StringView(filter)) {}
    template<typename ByteArray>
    NameFilter(const ByteArray &filter) :
      filter(
        reinterpret_cast<const uint8_t *>(filter.begin()),
        reinterpret_cast<const uint8_t *>(filter.end())
      ) {}
    NameFilter(const ByteBufferView &filter) : filter(filter) {}
    NameFilter(const NameFilter &filter) = delete; // prevent accidental copy-by-value

    // TopicFilter interface

    bool matches(const ByteBufferView &name) const {
      return name == filter;
    }

    bool prefixes(const ByteBufferView &name) const {
      return (
        (name.size() >= filter.size())
        && (ByteBufferView(name.data(), name.data() + filter.size()) == filter)
      );
    }

    ByteBufferView suffix(const ByteBufferView &name) const { // Warning: does not check whether the filter matches the prefix!
      if (filter.size() >= name.size()) return ByteBufferView();

      return ByteBufferView(name.begin() + filter.size(), name.end());
    }
};

// TopicEndpoint allows receiving and sending documents on one Endpoint with an exact endpoint name match

template<typename Document, Presentation::SerializationFormatCode Format>
class Endpoint {
  public:
    using Name = ByteBufferView;
    using ToReceive = Document; // The type of data passed up from below
    using Receive = Presentation::Document<Format>; // The type of data passed up to above
    using OptionalReceive = Util::Optional<Receive>;
    using Send = Presentation::Document<Format>;
    using SendDelegate = etl::delegate<bool(const Send &)>;
    using ToSend = Document; // The type of data passed down to below
    using ToSendDelegate = etl::delegate<bool(const ToSend &)>;

    const NameFilter filter;

    template<typename Filter>
    Endpoint(const Filter &filter) :
      filter(filter) {}
    template<typename Filter>
    Endpoint(const Filter &filter, const ToSendDelegate &delegate) :
      filter(filter), sender(&delegate) {}
    Endpoint(const Endpoint &endpoint) = delete;

    // Endpoint interface

    virtual ByteBufferView getEndpointName(const ToReceive &document) const = 0;
    virtual void prepareToSendDocument(ToSend &toSendDocument, const Send &sendDocument) const = 0;

    OptionalReceive receive(const ToReceive &document) {
      OptionalReceive received;
      if (!filter.matches(getEndpointName(document))) return received;
      return document;
    }
    bool send(const Send &sendDocument) {
      if (sender == nullptr) return false;

      Document toSendDocument;
      prepareToSendDocument(toSendDocument, sendDocument);
      return (*sender)(toSendDocument);
    }

    void setToSendDelegate(const ToSendDelegate &delegate) {
      sender = &delegate;
    }

  protected:
    const ToSendDelegate *sender;
};

// EndpointHandler is an interface class for a unit of the application which
// handles documents received on one or more Endpoints

template<typename Endpoint>
class EndpointHandler {
  public:
    using ToReceive = typename Endpoint::ToReceive; // The type of data passed up from below
    using ToSendDelegate = typename Endpoint::ToSendDelegate;
    using EndpointDocument = typename Endpoint::Receive;

    // Event loop interface
    virtual void setup() {}
    virtual void update() {}

    // Endpoint handler interface
    virtual void receive(const ToReceive &document) = 0;
    virtual void setToSendDelegate(const ToSendDelegate &delegate) = 0;
};

// SingleEndpointHandler is an abstact base class for a unit of the application which
// handles documents received on a single Endpoint

template<typename Endpoint>
class SingleEndpointHandler : public EndpointHandler<Endpoint> {
  public:
    using ToReceive = typename EndpointHandler<Endpoint>::ToReceive; // The type of data passed up from below
    using ToSendDelegate = typename EndpointHandler<Endpoint>::ToSendDelegate;
    using EndpointDocument = typename EndpointHandler<Endpoint>::EndpointDocument;

    template<typename Filter>
    SingleEndpointHandler(const Filter &filter) :
      endpoint(filter) {}
    template<typename Filter>
    SingleEndpointHandler(const Filter &filter, const ToSendDelegate &delegate) :
      endpoint(filter, delegate) {}
    SingleEndpointHandler(const SingleEndpointHandler &handler) = delete;

    // Endpoint handler interface

    void receive(const ToReceive &document) {
      auto topicReceived = endpoint.receive(document);
      if (!topicReceived) return;

      endpointReceived(*topicReceived);
    }
    void setToSendDelegate(const ToSendDelegate &delegate) {
      endpoint.setToSendDelegate(delegate);
    }

    // Single-endpoint handler interface

    virtual void endpointReceived(const EndpointDocument &document) {}

    bool send(const EndpointDocument &document) {
      return endpoint.send(document);
    }

  protected:
    Endpoint endpoint;
};

} } }