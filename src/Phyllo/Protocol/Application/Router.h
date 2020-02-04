#pragma once

// Standard libraries
#include <initializer_list>

// Third-party libaries
#include <etl/vector.h>

// Phyllo
#include "Phyllo/Types.h"
#include "Endpoint.h"

// Routers allow different objects to handle different documents depending on their respective named endpoints.

namespace Phyllo { namespace Protocol { namespace Application {

// Router allows automatic updating of endpoint handlers while the protocol stack is updated

template<typename Endpoint, size_t MaxHandlers = 256>
class Router : EndpointHandler<Endpoint> {
  public:
    using EndpointHandler = Application::EndpointHandler<Endpoint>;
    etl::vector<EndpointHandler *, MaxHandlers> handlers;

    Router() {}
    template<typename... HandlerTypes>
    Router(HandlerTypes&... handlers) : handlers() {
      using expand_type = int[];
      expand_type { (addHandler(handlers), 0)... };
    }

    // Event loop interface

    void setup() {
      for (auto &handler : handlers) handler->setup();
    }
    void update() {
      for (auto &handler : handlers) handler->update();
    }

    // Endpoint handler interface

    void receive(const typename Endpoint::ToReceive &document) {
      for (auto &handler : handlers) handler->receive(document);
    }
    void setToSendDelegate(const typename Endpoint::ToSendDelegate &delegate) {
      for (auto &handler : handlers) handler->setToSendDelegate(delegate);
    }

    // Router interface

    bool addHandler(EndpointHandler &handler) {
      if (handlers.full()) return false;

      handlers.push_back(&handler);
      return true;
    }
};

} } }