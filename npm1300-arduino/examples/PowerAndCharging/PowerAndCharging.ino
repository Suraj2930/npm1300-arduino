#include <Arduino.h>
#include <Wire.h>
#include <NPM1300.h>

NPM1300 pmic;

void setup() {

  Serial.begin(115200);
  Wire.begin();

  if (!pmic.begin()) {
    Serial.println("PMIC not found");
    while (1);
  }

  // Configure power rails
  pmic.writeBuck1(3.3);
  pmic.writeBuck2(3.3);

  // Configure charger
  pmic.disableCharger();
  pmic.setChargerIgnoreNTC(true);
  pmic.setChargerRechargeEnabled(true);
  pmic.writeChargeCurrent(100);
  pmic.writeChargeVoltage(4.15);
  pmic.enableCharger();

  Serial.println("Power rails and charger configured");
}

void loop() {
}