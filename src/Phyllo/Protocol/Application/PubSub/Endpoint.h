#pragma once

// Standard libraries

// Third-party libaries

// Phyllo
#include "Phyllo/Types.h"
#include "Phyllo/Protocol/Application/Endpoint.h"
#include "Phyllo/Protocol/Application/PubSub/Document.h"

// Endpoints allow different objects to handle different documents depending on their respective named endpoints.

namespace Phyllo { namespace Protocol { namespace Application { namespace PubSub {

// TopicEndpoint allows receiving and sending documents on one topic with an exact match

template<Presentation::SerializationFormatCode Format>
class Endpoint : public Application::Endpoint<Document<Format>, Format> {
  public:
    using EndpointInterface = Application::Endpoint<Document<Format>, Format>;

    template<typename Filter>
    Endpoint(const Filter &filter) :
      EndpointInterface(filter) {}
    template<typename Filter>
    Endpoint(const Filter &filter, const typename EndpointInterface::ToSendDelegate &delegate) :
      EndpointInterface(filter, delegate) {}
    Endpoint(const Endpoint &endpoint) = delete;

    // Endpoint interface

    ByteBufferView getEndpointName(const typename EndpointInterface::ToReceive &document) const {
      return document.topic();
    }

    void prepareToSendDocument(
      typename EndpointInterface::ToSend &toSendDocument, // Application::PubSub::Document<Formt>
      const typename EndpointInterface::Send &sendDocument // Prsentation::Document<Formt>
    ) const {
      toSendDocument.setTopic(this->filter.filter);
      toSendDocument.read(sendDocument.buffer());
    }
};

template<Presentation::SerializationFormatCode Format>
using EndpointHandler = Application::EndpointHandler<Endpoint<Format>>;

template<Presentation::SerializationFormatCode Format>
using SingleEndpointHandler = Application::SingleEndpointHandler<Endpoint<Format>>;

} } } }