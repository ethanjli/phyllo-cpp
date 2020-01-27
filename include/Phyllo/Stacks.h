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

// TODO: make a class which holds SerialLink, MediumStack, LogicalStack, and TransportStack as members

}