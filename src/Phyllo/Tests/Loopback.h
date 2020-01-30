#pragma once

// Standard libraries

// Third-party libraries
#include <etl/array.h>
#include <etl/cstring.h>
#include <elapsedMillis.h>

// Phyllo
#include "Phyllo/Types.h"
#include "Phyllo/Protocol/Presentation/Document.h"
#include "Phyllo/Protocol/Presentation/DocumentLink.h"
#include "Phyllo/Protocol/Presentation/MessagePack.h"
#include "Phyllo/Protocol/Application/Stacks.h"
#include "Phyllo/Protocol/Stacks.h"

// Utilities for loopback testing

namespace Phyllo { namespace Tests {

// Idling

template<typename ByteBufferLink>
void loopIdle(ByteBufferLink &link) { // useful to test whether background behavior works
}

// Constant payload

static const etl::array<uint8_t, 10> kTestPayload = {0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, '\n'};
static const etl::array<uint8_t, 5> kTestTopic = {"echo"};

template<typename ByteBufferLink>
void loopAnnounce(ByteBufferLink &link) { // useful to test sending correctness
  elapsedMillis timer;
  while (timer < 500) link.update();
  timer = 0;
  link.send(ByteBufferView(kTestPayload));
}

template<>
void loopAnnounce<Protocol::Presentation::MsgPack::DocumentLink>(
  Protocol::Presentation::MsgPack::DocumentLink &link
) { // useful to test sending correctness
  elapsedMillis timer;
  while (timer < 500) ;
  timer = 0;
  Protocol::Presentation::MsgPack::Document document;
  document.header.schema = 0xa0;
  document.writer.start();
  document.writer.startArray(2);
  document.writer.write("hello!");
  document.writer.write("world!", kTestPayload);
  document.writer.finishArray();
  document.writer.finish();
  link.send(document);
}

template<>
void loopAnnounce<Protocol::Application::PubSub::MsgPackDocumentLink>(
  Protocol::Application::PubSub::MsgPackDocumentLink &link
) { // useful to test sending correctness
  elapsedMillis timer;
  while (timer < 500) ;
  timer = 0;
  Protocol::Presentation::MsgPack::Document document;
  document.header.schema = 0xa0;
  document.writer.start();
  document.writer.startArray(2);
  document.writer.write("hello!");
  document.writer.write("world!", kTestPayload);
  document.writer.finishArray();
  document.writer.finish();
  link.send(ByteBufferView(kTestTopic), document);
}

template<typename Stack>
void loopReply(Stack &stack) { // useful to test whether receiving works
  auto stackReceived = stack.receive();
  if (stackReceived) stack.top.send(ByteBufferView(kTestPayload));
}

template<typename Stack>
void loopReplyDocument(Stack &stack) { // useful to test receiving correctness of DocumentLink
  using namespace Protocol::Presentation::MsgPack;

  stack.update();
  auto stackReceived = stack.receive();
  if (!stackReceived) return;

  Document document;
  document.header.schema = 0xa0;
  Document::Writer &writer = document.writer;
  writer.start();
  writer.write("hello!");
  writer.finish();
  stack.top.send(document);
}

Protocol::Presentation::MsgPack::Document makeDocument() {
  using namespace Protocol::Presentation::MsgPack;

  Document document;
  document.header.schema = 0xa0;
  Document::Writer &writer = document.writer;
  writer.start();
  writer.write("hello!");
  writer.finish();
  return document;
}

template<typename Stack>
void loopReplyMessage(Stack &stack) { // useful to test receiving correctness of DocumentLink
  stack.update();
  auto stackReceived = stack.receive();
  if (!stackReceived) return;

  stack.top.send(ByteBufferView(kTestTopic), makeDocument());
}

// Echo

template<typename TransportStack>
void loopEchoPayload(TransportStack &stack) { // useful to test receiving correctness
  stack.update();
  auto stackReceived = stack.receive();
  if (stackReceived) stack.top.send(getPayload(*stackReceived));
}

template<typename Stack>
void loopEchoDirect(Stack &stack) { // useful to test receiving correctness of connectionless layers
  stack.update();
  auto stackReceived = stack.receive();
  if (stackReceived) stack.top.send(*stackReceived);
}

Protocol::Presentation::MsgPack::Document copyFlatDocument(
  const Protocol::Presentation::MsgPack::Document &inDocument
) { // warning: only valid for primitives, and arrays and maps without nesting!
  using namespace Protocol::Presentation::MsgPack;

  Document document;
  document.header.schema = inDocument.header.schema;
  Document::Reader &reader = inDocument.reader;
  Document::Writer &writer = document.writer;
  reader.start();
  writer.start();
  bool rootArray = reader.isArray();
  bool rootMap = reader.isMap();
  size_t length;
  if (rootArray) {
    length = reader.startArray();
    writer.startArray(length);
  } else if (rootMap) {
    length = reader.startMap();
    writer.startMap(length);
    length *= 2;
  } else {
    length = 1;
  }
  for (size_t i = 0; i < length; ++i) {
    switch (reader.peekType()) {
      case FieldType::Boolean:
        writer.write(reader.read<bool>());
        break;
      case FieldType::Uint:
        writer.write(reader.read<unsigned int>());
        break;
      case FieldType::Int:
        writer.write(reader.read<int>());
        break;
      case FieldType::Float32:
        writer.write(reader.read<float>());
        break;
      case FieldType::Float64:
        writer.write(reader.read<double>());
        break;
      case FieldType::String:
        writer.write(reader.read<StringView>());
        break;
      case FieldType::Binary:
        writer.write(reader.read<BinaryView>());
        break;
      default:
        reader.discard();
        writer.writeNone();
    }
  }
  if (rootArray) {
    reader.finishArray();
    writer.finishArray();
  } else if (rootMap) {
    reader.finishMap();
    writer.finishMap();
  }
  reader.finish();
  writer.finish();
  return document;
}

template<typename Stack>
void loopEchoFlat(Stack &stack) { // useful to test receiving correctness of DocumentLink
  stack.update();
  auto stackReceived = stack.receive();
  if (!stackReceived) return;

  stack.top.send(copyFlatDocument(*stackReceived));
}
template<typename Stack>
void loopEchoFlatMessage(Stack &stack) { // useful to test receiving correctness of DocumentLink
  stack.update();
  auto stackReceived = stack.receive();
  if (!stackReceived) return;

  stack.top.send(stackReceived->topic(), copyFlatDocument(*stackReceived));
}

// Counter

class Counter {
  public:
    uint8_t value = 0;

    void increment() {
      ++value;
    }
};

} }