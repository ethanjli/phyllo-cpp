#pragma once

// Standard libraries

// Third-party libraries
#include <elapsedMillis.h>
#include <etl/delegate.h>

// Phyllo

namespace Phyllo { namespace Util {

using TimerTask = etl::delegate<void(void)>;

class TimeoutTimer {
  public:
    bool enabled = false;
    unsigned long timeout = 0;

    TimeoutTimer() {}

    TimeoutTimer(unsigned long timeout) :
      timeout(timeout) {}

    void start(unsigned long timeout) { // set timeout and start timer from beginning
      this->timeout = timeout;
      start();
    }
    void start() { // start timer from beginning
      enabled = true;
      reset();
    }

    void reset() {
      clock = 0;
    }

    void resetAndStop() {
      enabled = false;
      reset();
    }

    bool running() const {
      return enabled && (clock < timeout);
    }

    bool timedOut() const {
      return enabled && (clock >= timeout);
    }

    unsigned long remaining() const {
      if (!enabled) return timeout;

      unsigned long duration = timeout - clock;
      if (duration > 0) return duration;
      else return 0;
    }

  protected:
    elapsedMillis clock;
};

class TimeoutTask {
  public:
    TimeoutTimer timer;

    TimeoutTask() : TimeoutTask(0) {}

    TimeoutTask(unsigned long timeout) :
      timer(timeout) {}

    virtual void setup() {}

    void setTask(const TimerTask &task) {
      this->task = task;
    }

    void update() {
      if (!timer.enabled) return;
      if (timer.running()) return;

      task();
      timer.resetAndStop();
    }

  protected:
    TimerTask task;
};

} }