#pragma once

// Standard libraries
#include <initializer_list>

// Third-party libaries
#include <etl/vector.h>

// Phyllo
#include "Phyllo/Types.h"
#include "Phyllo/Protocol/Application/Router.h"

// Routers allow different objects to handle different documents depending on their respective named endpoints.

namespace Phyllo { namespace Protocol { namespace Application { namespace PubSub {

// Router allows automatic updating of endpoint handlers while the protocol stack is updated

template<Presentation::SerializationFormatCode Format, size_t MaxHandlers = 127>
using Router = Application::Router<Endpoint<Format>>;

} } } }