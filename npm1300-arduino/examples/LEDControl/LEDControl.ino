#include <Arduino.h>
#include <Wire.h>
#include <NPM1300.h>

NPM1300 pmic;

void setup() {

  Serial.begin(115200);
  Wire.begin();

  pmic.begin();

  pmic.setAllLedsHostMode();
}

void loop() {

  uint8_t status = pmic.readChgStatusRaw();

  bool charging = status & (1 << 2);
  bool done     = status & (1 << 1);

  pmic.ledOff(0);
  pmic.ledOff(1);
  pmic.ledOff(2);

  if (charging) pmic.ledOn(0);
  if (done) pmic.ledOn(1);

  delay(500);
}