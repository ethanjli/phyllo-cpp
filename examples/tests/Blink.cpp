// Test whether a blink sketch correctly compiles and uploads when
// the Phyllo header files needed for the echo test are included.

// Standard libraries
#include <Arduino.h>

// Third-party libraries

// Phyllo
#include "Phyllo.h"
#include "Phyllo/Tests/Loopback.h"

// Arduino

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop() {
  delay(1000);
  digitalWrite(LED_BUILTIN, HIGH);
  delay(1000);
  digitalWrite(LED_BUILTIN, LOW);
}