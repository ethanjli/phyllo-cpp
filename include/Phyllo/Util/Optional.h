#pragma once

// Standard libraries

// Third-party libraries

// Phyllo

// Optionals are similar ETL optionals, but faster due to never calling destructors
// and not needing to be initialized with a new default constructor to enable it.
// The value member can always be written to, whether or not the optional is enabled.

namespace Phyllo { namespace Util {

template<typename T>
class Optional {
  public:
    bool enabled;
    T value;

    Optional(const T &value) :
      enabled(true), value(value) {}
    Optional(const Optional<T> &other) :
      enabled(other.enabled), value(other.value) {}
    Optional() :
      enabled(false), value() {}

    explicit operator bool() const {
      return enabled;
    }

    void enable() {
      enabled = true;
    }
    void clear() {
      enabled = false;
    }

    Optional& operator=(const Optional<T>& value) {
      enabled = value.enabled;
      this->value = value.value;
      return *this;
    }
    Optional& operator=(const T& value) {
      enabled = true;
      this->value = value;
      return *this;
    }

    const T* operator->() const {
      return &value;
    }
    T* operator->() {
      return &value;
    }

    const T& operator*() const {
      return value;
    }
    T& operator*() {
      return value;
    }
};

} }