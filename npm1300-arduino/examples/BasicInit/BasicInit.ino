#include <Arduino.h>
#include <Wire.h>
#include <NPM1300.h>

NPM1300 pmic;

void setup() {
  Serial.begin(115200);
  Wire.begin();

  if (!pmic.begin()) {
    Serial.println("nPM1300 not detected");
    while (1);
  }

  Serial.println("nPM1300 initialized");
}

void loop() {
}