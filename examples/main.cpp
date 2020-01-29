// Standard libraries
#include <Arduino.h>

// Third-party libraries

// Phyllo
#include "Phyllo/Platform.h"
#include "Phyllo/Stacks.h"

//using TransportStack = Phyllo::MinimalTransportStack;
using TransportStack = Phyllo::ReducedTransportStack;
//using TransportStack = Phyllo::StandardTransportStack;
using ProtocolStack = Phyllo::Protocol::MinimalStack<TransportStack>;

#if PHYLLO_SERIAL == PHYLLO_SERIAL_UART
using SerialLink = Phyllo::IO::SerialUARTLink;
const long kSerialDataRate = Phyllo::IO::kSerialUARTDataRate;
#elif PHYLLO_SERIAL == PHYLLO_SERIAL_NATIVE || PHYLLO_SERIAL == PHYLLO_SERIAL_TEENSY
using SerialLink = Phyllo::IO::SerialUSBLink;
const long kSerialDataRate = Phyllo::IO::kSerialUSBDataRate;
#endif

SerialLink serialLink(kSerialDataRate);
TransportStack transportStack(serialLink);
ProtocolStack protocolStack(transportStack);
//Phyllo::Protocol::Transport::CommandLink commandLink(objectLink);

/*
namespace Verb = Phyllo::Verb;
using Command = Phyllo::Command;

class TimeoutCommand : public Phyllo::TimeoutTask {
  public:
    Phyllo::Command command;
    Phyllo::CommandLink &commandLink;

    TimeoutCommand (Phyllo::CommandLink &commandLink) :
      commandLink(commandLink) {}

    void setup() {
      setTask(Phyllo::TimerTask::create<TimeoutCommand, &TimeoutCommand::sendCommand>(*this));
    }

    void sendCommand() {
      commandLink.sendCommand(command);
    }
};
*/

//TimeoutCommand coreTimer(commandLink);

// Arduino

//auto &commLink = transportStack.top; // test the transport stack
auto &commLink = protocolStack.top; // test the protocol stack

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  protocolStack.setup();
  //coreTimer.setup();
}


/*
void loopCommandLink()
{
  commandLink.update();
  coreTimer.update();

  if (!commandLink.received) return;

  Phyllo::Command &receivedCommand = commandLink.receivedCommand;
  Phyllo::Command::NamePath &name = receivedCommand.name;
  Phyllo::Command::Id id = receivedCommand.id;
  if (receivedCommand.verb == Verb::read && name[0] == 0x01 && name[1] == 0x03) { // core/time
    commandLink.sendCommand(Command(Verb::info, name, id, millis()));
  } else if (name[0] == 0x01 && name[1] == 0x04) { // core/timer
    if (receivedCommand.verb == Verb::call) {
      unsigned long input = receivedCommand.args;
      commandLink.sendCommand(Command(Verb::await, name, id, input));
      coreTimer.command.set(Verb::info, name, id, 0);
      coreTimer.timer.start(input);
    } else if (receivedCommand.verb == Verb::read) {
      if (coreTimer.timer.enabled) {
        commandLink.sendCommand(Command(Verb::info, name, id, coreTimer.timer.remaining()));
      } else {
        commandLink.sendCommand(Command(Verb::info, name, id));
      }
    } else {
      commandLink.sendCommand(Command(Verb::none, name, id));
    }
  } else if (receivedCommand.verb == Verb::call && name[0] == 0x02 && name[1] == 0x02) { // comm/echo
    commandLink.sendCommand(Command(Verb::info, name, id, receivedCommand.args));
  } else if (receivedCommand.verb == Verb::call && name[0] == 0x02 && name[1] == 0x03) { // comm/ping
    unsigned int ping_value = receivedCommand.args;
    commandLink.sendCommand(Command(Verb::info, name, id, ping_value + 1));
  } else if (receivedCommand.verb == Verb::call && name[0] == 0x02 && name[1] == 0x04) { // comm/len
    if (receivedCommand.args.is<JsonArray>()) {
      commandLink.sendCommand(Command(Verb::info, name, id, receivedCommand.args.size()));
    } else if (receivedCommand.args.is<const char *>()) {
      const char *input = receivedCommand.args;
      commandLink.sendCommand(Command(Verb::info, name, id, strlen(input)));
    } else {
      commandLink.sendCommand(Command(Verb::none, name, id));
    }
  }  else if (receivedCommand.verb == Verb::call && name[0] == 0x02 && name[1] == 0x05) { // comm/crc
    const char *input = receivedCommand.args;
    commandLink.sendCommand(Command(Verb::info, name, id, Phyllo::reflectedCRC32sub8(reinterpret_cast<const uint8_t *>(input), strlen(input))));
  } else {
    commandLink.sendCommand(Command(Verb::none, name, id));
  }
}
*/

void loop() {
  // Application tests
  // loopCommandLink();
}