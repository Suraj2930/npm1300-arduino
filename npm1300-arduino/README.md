# nPM1300 Arduino Library

Arduino-compatible library for the **Nordic nPM1300 Power Management IC (PMIC)** with I²C drivers for battery charging, power rails, monitoring, fuel gauge estimation, and GPIO/LED control.

The library provides a clean abstraction over the nPM1300 register map, allowing firmware to control the PMIC using simple APIs instead of raw register writes.

Compatible with Arduino-based platforms such as **nRF52, ESP32, STM32, RP2040**, and other microcontrollers with I²C support.

---

# Features

- ⚡ Buck regulator control (BUCK1 / BUCK2)
- 🔋 Battery charging management
- 🔌 VBUS / VBAT / VSYS voltage monitoring
- 📊 Battery State-of-Charge (SOC) estimation
- ⛽ Simple coulomb-counting fuel gauge
- 🌡 Die temperature measurement
- 🔆 LED driver control (LEDDRV0..2)
- 🔧 GPIO input/output with pull-up / pull-down
- 🔋 LDO / Load-switch control
- 🔍 Charger state and error decoding
- 🧰 Arduino-friendly API

---

# Supported Hardware

This library targets the:

**Nordic Semiconductor nPM1300 PMIC**

Typical MCU pairings include:

- nRF52840
- nRF52832
- ESP32
- RP2040
- STM32
- Any Arduino-compatible board with I²C

---

# Installation

## Arduino IDE

1. Download or clone this repository
2. Place it inside:

```
Arduino/libraries/
```

3. Restart the Arduino IDE

---

## PlatformIO

Add the library to your project:

```
lib_deps =
    npm1300-arduino
```

---

# Basic Example

Minimal setup demonstrating power rails, charger configuration, and monitoring.

```cpp
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

  // Enable power rails
  pmic.setBuck1Voltage(3.3);
  pmic.enableBuck1();

  pmic.setBuck2Voltage(1.8);
  pmic.enableBuck2();

  // Configure charger
  pmic.setChargeVoltage(4.2);
  pmic.setChargeCurrent(500);
  pmic.enableCharger();
}

void loop() {

  Serial.print("VBAT: ");
  Serial.print(pmic.getVbatVoltage());
  Serial.println(" mV");

  delay(1000);
}
```

---

# API Overview

## Initialization

```
pmic.begin()
pmic.isConnected()
```

---

## Buck Regulators

```
pmic.enableBuck1()
pmic.enableBuck2()

pmic.disableBuck1()
pmic.disableBuck2()

pmic.setBuck1Voltage(voltage)
pmic.setBuck2Voltage(voltage)
```

---

## Charger Control

```
pmic.enableCharger()
pmic.disableCharger()

pmic.setChargeCurrent(mA)
pmic.setChargeVoltage(voltage)

pmic.setChargerIgnoreNTC(bool)
pmic.setChargerRechargeEnabled(bool)
```

---

## Battery Monitoring

```
pmic.getVbusVoltage()
pmic.getVbatVoltage()
pmic.getVsysVoltage()

pmic.getBatterySOC()
```

---

## Charging Status

```
pmic.isCharging()
pmic.isBatteryFull()

pmic.isTrickleCharging()
pmic.isConstantCurrentCharging()
pmic.isConstantVoltageCharging()
```

---

## Fuel Gauge

```
pmic.fuelGaugeBegin(capacity_mAh)
pmic.fuelGaugeUpdate()

pmic.fuelGaugeSoc()
pmic.fuelGaugeRemaining_mAh()

pmic.fuelGaugeTTE_min()
pmic.fuelGaugeTTF_min()
```

---

## LED Driver

```
pmic.setLedMode(led, mode)

pmic.ledOn(led)
pmic.ledOff(led)

pmic.setAllLedsHostMode()
```

---

## GPIO Control

```
pmic.setGpioMode(pin, mode)
pmic.setGpioOutput(pin, state)

pmic.getGpioInput(pin)

pmic.setGpioPullUp(pin, enable)
pmic.setGpioPullDown(pin, enable)
```

---

## LDO / Load Switch

```
pmic.selectLdo1Mode(true)
pmic.selectLdo2Mode(true)

pmic.setLdo1Voltage(voltage)
pmic.setLdo2Voltage(voltage)

pmic.enableLdo1()
pmic.enableLdo2()
```

---

# Examples

This repository includes multiple examples demonstrating different features:

- Basic Initialization
- Power Rails
- Charger Control
- Battery Monitoring
- LED Control
- GPIO Example
- Fuel Gauge
- Temperature Monitoring

---

# Why This Library?

Many nPM1300 integrations rely on raw register access scattered throughout firmware.

This library instead provides:

- A structured driver
- Readable APIs
- Reusable charger and status logic
- Cleaner firmware architecture

---

# Contributing

Pull requests and improvements are welcome.  
If you find issues or have feature requests, please open an issue.

# License

MIT License

# Author

Created and maintained by Jay Patel.