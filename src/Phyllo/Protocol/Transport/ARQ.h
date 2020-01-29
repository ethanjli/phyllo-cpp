#pragma once

// Standard libraries

// Third-party libraries
#include <etl/queue.h>
#include <etl/delegate.h>

// Phyllo
#include "Phyllo/Types.h"
#include "Phyllo/Util/Timing.h"
#include "ReliableBuffer.h"
#include "DatagramLink.h"

// WARNING: implementation is incomplete!

namespace Phyllo { namespace Protocol { namespace Transport {

class GBNSender {
  public:
    static const size_t kReceiverWindowSize = 1;  // implicit in algorithm implementation
    static const size_t kSenderWindowSize = 8;
    static const size_t kSendQueueSize = 8;
    static const size_t kSequenceNumberSpace = 256;
    static_assert(
      kSenderWindowSize <= kSequenceNumberSpace - kReceiverWindowSize,
      "Sum of sender window size and receiver window size cannot exceed size of sequence number space"
    );

    // Event loop interface

    void setup() {}
    void update() {}

    // ARQReceiver interface

    void receive(const ReliableBuffer &reliableBuffer) {
      // TODO: check the flags to see whether we need to re-send an in-flight reliableBuffer
    }
    
    // ARQSender interface

    bool readyToSend() const {
      return false;
    }

    const ReliableBuffer &reliableBufferToSend() const {
      return sendQueue.front();
    }

    bool readyToEnqueue() const {
      return !sendQueue.full();
    }

    bool enqueue(const ReliableBuffer &reliableBuffer) {
      if (sendQueue.full()) return false;
      sendQueue.push(reliableBuffer);
      return true;
    }

  protected:
    ReliableBufferHeader::SequenceNumber seqNumMin = 0;
    ReliableBufferHeader::SequenceNumber seqNumMax = 0;

    etl::queue<ReliableBuffer, kSendQueueSize, etl::memory_model::MEMORY_MODEL_SMALL> sendQueue;
};

class GBNReceiver {
  public:
    static const size_t kReceiverWindowSize = 1;  // implicit in algorithm implementation
    static const unsigned int kPiggybackTimeout = 4; // ms

    using ToSend = ByteBufferView;
    using ToSendDelegate = etl::delegate<bool(const ToSend &, DataUnitTypeCode)>;

    GBNReceiver(const ToSendDelegate &delegate) :
      piggybackSender(kPiggybackTimeout), sender(delegate) {}

    // Event loop interface

    void setup() {
      piggybackSender.setTask(
        Util::TimerTask::create<GBNReceiver, &GBNReceiver::sendRequest>(*this)
      );
    }

    void update() {
      piggybackSender.update();
    }

    // GBNReceiver interface

    bool receive(const ReliableBuffer &reliableBuffer) {
      bool reliableBufferReceived = (reliableBuffer.header.seqNum == nextExpected); // TODO: use the nos field
      //reliableBufferReceived = true; // debugging test
      //if (reliableBuffer.header.seqNum > 2 && !sentNAK) reliableBufferReceived = false; // debugging test
      if (reliableBufferReceived) {
        ++nextExpected;
        sendNAK = false;
        sentNAK = false;
      } else {
        sendNAK = true; // because we got an unexpected reliableBuffer, so request retransmission with NAK exactly once
      }

      piggybackSender.timer.start();
      return reliableBufferReceived;
    }

    void prepare(ReliableBufferHeader &reliableBufferHeader) const {
      reliableBufferHeader.ackNum = nextExpected;
      reliableBufferHeader.flags.value.ack = true;
      reliableBufferHeader.flags.value.nak = sendNAK && !sentNAK;
    }

    void sent(const ReliableBufferHeader &reliableBufferHeader) {
      piggybackSender.timer.resetAndStop();
      if (reliableBufferHeader.flags.value.nak) sentNAK = true;
    }

  protected:
    // Acknowledgements
    ReliableBufferHeader::SequenceNumber nextExpected = 0;
    bool sendNAK = false;
    bool sentNAK = false;
    Util::TimeoutTask piggybackSender;
    bool &issueRequest = piggybackSender.timer.enabled;

    const ToSendDelegate &sender;

    void sendRequest() { // TODO: actually, should we just expose readyToSend and reliableBufferToSend for reliableBufferLink to send? We do want to bypass arqSender's queue by sending it as an unreliable reliableBuffer
      ReliableBuffer reliableBuffer;
      prepare(reliableBuffer.header);
      reliableBuffer.header.flags.value.nos = true;
      reliableBuffer.header.type = DataUnitType::Layer::Control;
      reliableBuffer.writeEmpty();
      if (!sender(reliableBuffer.buffer(), DataUnitType::Transport::ReliableBuffer)) return;
      // TODO: handle failure to write reliableBuffer or send frame with a frame-level error signal
      sent(reliableBuffer.header);
    }
};

} } }