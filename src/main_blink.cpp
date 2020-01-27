// Standard libraries
#include <math.h>
#include <Arduino.h>

// Phyllo platform configuration
#include "Phyllo/Platform.h"

// Third-party libraries

// Phyllo
#include "Phyllo/Stacks.h"
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