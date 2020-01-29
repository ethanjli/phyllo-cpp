// Standard libraries
#include <Arduino.h>

// Phyllo platform configuration
#include "Phyllo/Platform.h"

// Third-party libraries

// Phyllo
#include "Phyllo/Stacks.h"
#include "Phyllo/Tests/Loopback.h"

// Choose conventional stack configurations

// Serial Port configuration:
auto &SerialStream = Phyllo::IO::USBSerial; // automatically chosen based on platform
//auto &SerialStream = Serial; // Default USB on Arduino & Teensy boards
//auto &SerialStream = SerialUSB; // Native USB on Due & Zero boards
//auto &SerialStream = Serial1; // Hardware Serial
//auto &SerialStream = Serial2; // Hardware Serial
//auto &SerialStream = Serial3; // Hardware Serial
//auto &SerialStream = Serial4; // Hardware Serial
//auto &SerialStream = Serial5; // Hardware Serial
//auto &SerialStream = Serial6; // Hardware Serial

// Serial Port Data Rate configuration (ignored for Due on Native USB port, Micro, Leonardo, and Teensy):
static const long kUSBSerialRate = Phyllo::IO::kUSBSerialRate; // automatically chosen by build flag, defaults to 115200

// I/O + Transport Medium Sub-Stack configuration:
using MediumStack = Phyllo::SerialMediumStack;

// Transport Logical Sub-Stack configuration:
using LogicalStack = Phyllo::Protocol::Transport::MinimalLogicalStack;
//using LogicalStack = Phyllo::Protocol::Transport::ReducedLogicalStack;
//using LogicalStack = Phyllo::Protocol::Transport::StandardLogicalStack;

// Application Stack configuration:
//using ApplicationStack = Phyllo::Protocol::Application::MinimalStack;
using ApplicationStack = Phyllo::Protocol::Application::PubSubStack;

// Stack types automatically deduced

using TransportStack = Phyllo::Protocol::Transport::TransportStack<MediumStack, LogicalStack>;
using ProtocolStack = Phyllo::Protocol::ProtocolStack<TransportStack, ApplicationStack>;

// Stack classes automatically created

// TODO: use the configurations to make a templated complete stack which owns and initializes all of the following:
MediumStack mediumStack(SerialStream);
LogicalStack logicalStack(mediumStack.sender);
TransportStack transportStack(mediumStack, logicalStack);
ApplicationStack applicationStack(transportStack.sender);
ProtocolStack protocolStack(transportStack, applicationStack);

// Alias configuration:

//auto &outerStack = transportStack;
auto &outerStack = protocolStack;

auto &commLink = outerStack.top;

// Arduino

void setup()
{
  Phyllo::IO::startSerial(SerialStream, kUSBSerialRate);
  pinMode(LED_BUILTIN, OUTPUT);
  outerStack.setup();
}

void loop() {
  // Transport stack tests
  //Phyllo::Tests::loopAnnounce(transportStack);
  //Phyllo::Tests::loopReply(transportStack);
  //Phyllo::Tests::loopEchoPayload(transportStack);

  // Application stack tests
  //Phyllo::Tests::loopAnnounce(protocolStack.top);
  //Phyllo::Tests::loopReplyDocument(protocolStack);
  //Phyllo::Tests::loopEchoDirect(protocolStack);
  //Phyllo::Tests::loopEchoFlat(protocolStack); // For MinimalStack

  // Pub-Sub Messaging tests
  //Phyllo::Tests::loopAnnounce(protocolStack.top);
  //Phyllo::Tests::loopReplyMessage(protocolStack);
  //Phyllo::Tests::loopEchoDirect(protocolStack);
  Phyllo::Tests::loopEchoFlatMessage(protocolStack); // For PubSubStack
}