#include <Arduino.h>
#include <Wire.h>
#include <NPM1300.h>

NPM1300 pmic;

void setup() {

  Serial.begin(115200);
  Wire.begin();

  pmic.begin();
}

void loop() {

  Serial.print("VBAT: ");
  Serial.print(pmic.readVBAT());
  Serial.print(" mV  ");

  Serial.print("VBUS: ");
  Serial.print(pmic.readVBUS());
  Serial.print(" mV  ");

  Serial.print("VSYS: ");
  Serial.print(pmic.readVSYS());
  Serial.print(" mV  ");

  Serial.print("SOC: ");
  Serial.print(pmic.readSOC());
  Serial.println(" %");

  delay(1000);
}