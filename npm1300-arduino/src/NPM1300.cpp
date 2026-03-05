#include "NPM1300.h"

NPM1300::NPM1300(TwoWire &wirePort) { _i2c = &wirePort; }

bool NPM1300::begin()
{
  _i2c->begin();
  return isConnected();
}

bool NPM1300::isConnected()
{
  _i2c->beginTransmission(npm1300::I2C_ADDR);
  return (_i2c->endTransmission() == 0);
}

// ===== BUCK =====
bool NPM1300::enableBuck1() { return writeRegister(npm1300::reg::BUCK1_ENASET, 0x01); }
bool NPM1300::enableBuck2() { return writeRegister(npm1300::reg::BUCK2_ENASET, 0x01); }
bool NPM1300::disableBuck1() { return writeRegister(npm1300::reg::BUCK1_ENACLR, 0x01); }
bool NPM1300::disableBuck2() { return writeRegister(npm1300::reg::BUCK2_ENACLR, 0x01); }

bool NPM1300::setBuck1Voltage(float voltage)
{
  uint8_t regValue = voltageToRegValue(voltage);
  if (!writeRegister(npm1300::reg::BUCK1_NORMVOUT, regValue))
    return false;

  uint8_t swCtrl = 0;
  if (!readRegister(npm1300::reg::BUCKSWCTRLSEL, &swCtrl))
    return false;

  swCtrl |= 0x01;
  return writeRegister(npm1300::reg::BUCKSWCTRLSEL, swCtrl);
}

bool NPM1300::setBuck2Voltage(float voltage)
{
  uint8_t regValue = voltageToRegValue(voltage);
  if (!writeRegister(npm1300::reg::BUCK2_NORMVOUT, regValue))
    return false;

  uint8_t swCtrl = 0;
  if (!readRegister(npm1300::reg::BUCKSWCTRLSEL, &swCtrl))
    return false;

  swCtrl |= 0x02;
  return writeRegister(npm1300::reg::BUCKSWCTRLSEL, swCtrl);
}

// ===== ADC Voltages, Current & Temperature =====
uint16_t NPM1300::getVbusVoltage()
{
  writeRegister(npm1300::reg::TASK_VBUS_MEAS, 0x01);
  delay(1);

  uint8_t msb = 0, lsb = 0;
  if (readRegister(npm1300::reg::ADC_VBAT3_MSB, &msb) &&
      readRegister(npm1300::reg::ADC_GP1_LSBS, &lsb))
  {
    uint16_t raw = (uint16_t(msb) << 2) | ((lsb >> 6) & 0x03);
    return (raw * 7500UL) / 1023;
  }
  return 0;
}

uint16_t NPM1300::getVbatVoltage()
{
  writeRegister(npm1300::reg::TASK_VBAT_MEAS, 0x01);
  delay(1);

  uint8_t msb = 0, lsb = 0;
  if (readRegister(npm1300::reg::ADC_VBAT_MSB, &msb) &&
      readRegister(npm1300::reg::ADC_GP0_LSBS, &lsb))
  {
    uint16_t raw = (uint16_t(msb) << 2) | (lsb & 0x03);
    return (raw * 5000UL) / 1023;
  }
  return 0;
}

uint16_t NPM1300::getVsysVoltage()
{
  writeRegister(npm1300::reg::TASK_VSYS_MEAS, 0x01);
  delay(1);

  uint8_t msb = 0, lsb = 0;
  if (readRegister(npm1300::reg::ADC_VSYS_MSB, &msb) &&
      readRegister(npm1300::reg::ADC_GP0_LSBS, &lsb))
  {
    uint16_t raw = (uint16_t(msb) << 2) | ((lsb >> 6) & 0x03);
    return (raw * 6375UL) / 1023;
  }
  return 0;
}

bool NPM1300::enableAutoIBAT(bool en)
{
  return writeRegister(npm1300::reg::ADC_IBAT_EN, en ? 1 : 0);
}

uint8_t NPM1300::readIBATStatusRaw()
{
  uint8_t s = 0;
  readRegister(npm1300::reg::ADC_IBAT_STATUS, &s);
  return s;
}

static inline uint16_t adc10_from_msb_lsbs(uint8_t msb, uint8_t lsbs_2bit)
{
  return (uint16_t(msb) << 2) | (lsbs_2bit & 0x03);
}

int16_t NPM1300::getIBATmA()
{
  // You MUST have ADC_IBAT_EN enabled for this to work:
  // ADCIBATMEASEN=1 => IBAT is measured automatically after VBAT task.
  // Trigger VBAT single-shot; IBAT is captured right after.
  if (!writeRegister(npm1300::reg::TASK_VBAT_MEAS, 0x01))
    return 0;

  delay(2); // give it a bit more than 1ms; measurement+store can take longer

  // Read status (invalid flag + direction)
  uint8_t st = 0;
  if (!readRegister(npm1300::reg::ADC_IBAT_STATUS, &st))
    return 0;

  const bool invalid = (st & (1u << 4)) != 0; // IBATMEASEINVALID
  const uint8_t mode = (st >> 2) & 0x03;      // BCHARGERMODE: 01 discharge, 11 charge (per datasheet)

  if (invalid)
    return 0;

  // Read IBAT ADC result: VBAT2 burst slot holds IBAT result when auto-IBAT enabled.
  uint8_t msb = 0, gp1 = 0;
  if (!readRegister(npm1300::reg::ADC_VBAT2_MSB, &msb) ||
      !readRegister(npm1300::reg::ADC_GP1_LSBS, &gp1))
    return 0;

  // GP1 layout: [7:6]=VBAT3 LSBs, [5:4]=VBAT2 LSBs, [3:2]=VBAT1, [1:0]=VBAT0
  const uint8_t lsb2 = (gp1 >> 4) & 0x03;                // VBAT2RESULTLSB
  const uint16_t raw10 = adc10_from_msb_lsbs(msb, lsb2); // 0..1023
  const float frac = float(raw10) / 1023.0f;

  // Compute full-scale current from programmed charger limits (datasheet text)
  // discharge full-scale = (ISETDISCHARGE) * 0.836 mA
  // charge full-scale    = (ISETCHARGE)    * 1.25  mA
  uint8_t is_msb = 0, is_lsb = 0;
  uint16_t iset = 0;
  float fullScale_mA = 0.0f;

  if (mode == 0x01) // discharge
  {
    if (!readRegister(npm1300::reg::BCHG_ISETDISCHARGEMSB, &is_msb) ||
        !readRegister(npm1300::reg::BCHG_ISETDISCHARGELSB, &is_lsb))
      return 0;

    iset = (uint16_t(is_msb) << 1) | (is_lsb & 0x01);
    fullScale_mA = float(iset) * 0.836f;

    // measured current is negative during discharge
    return (int16_t)lroundf(-frac * fullScale_mA);
  }
  else if (mode == 0x03) // charge
  {
    if (!readRegister(npm1300::reg::BCHG_ISETMSB, &is_msb) ||
        !readRegister(npm1300::reg::BCHG_ISETLSB, &is_lsb))
      return 0;

    iset = (uint16_t(is_msb) << 1) | (is_lsb & 0x01);
    fullScale_mA = float(iset) * 1.25f;

    return (int16_t)lroundf(frac * fullScale_mA);
  }

  // idle/unknown
  return 0;
}

uint16_t NPM1300::getDieTempRaw()
{
  writeRegister(npm1300::reg::TASK_DIETEMP_MEAS, 0x01);
  delay(1);

  uint8_t msb = 0, lsb = 0;

  if (readRegister(npm1300::reg::ADC_DIETEMP_MSB, &msb) &&
      readRegister(npm1300::reg::ADC_GP0_LSBS, &lsb))
  {
    // TEMP LSBs are GP0[5:4]
    return (uint16_t(msb) << 2) | ((lsb >> 4) & 0x03);
  }
  return 0;
}

float NPM1300::getDieTempC()
{
  const uint16_t k = getDieTempRaw();
  if (k == 0)
    return NAN; // avoid printing garbage if read failed
                // TD = 394.67 - 0.7926 * K_DIETEMP
  return 394.67f - 0.7926f * float(k);
}

// ===== Charger state =====
uint8_t NPM1300::readChgStatusRaw()
{
  uint8_t s = 0;
  readRegister(npm1300::reg::BCHG_CHARGESTATUS, &s);
  return s;
}

uint8_t NPM1300::readChgErrReasonRaw()
{
  uint8_t e = 0;
  readRegister(npm1300::reg::BCHG_ERRREASON, &e);
  return e;
}

bool NPM1300::isCharging()
{
  uint8_t s = readChgStatusRaw();
  return (s & ((1u << 2) | (1u << 3) | (1u << 4))) != 0;
}

bool NPM1300::isBatteryFull()
{
  return (readChgStatusRaw() & (1u << 1)) != 0;
}

// ===== SOC (voltage-based estimate) =====
uint8_t NPM1300::getBatterySOC()
{
  const uint16_t v = getVbatVoltage();
  const uint16_t vEmpty = _socEmpty_mV;
  const uint16_t vFull = _vterm_mV;

  if (v <= vEmpty)
    return 0;
  if (v >= vFull)
    return 100;

  float x = float(v - vEmpty) / float(vFull - vEmpty);

  // Simple piecewise curve (better than linear)
  float soc;
  if (x < 0.15f)
    soc = x / 0.15f * 12.0f;
  else if (x < 0.85f)
    soc = 12.0f + (x - 0.15f) / 0.70f * 78.0f;
  else
    soc = 90.0f + (x - 0.85f) / 0.15f * 10.0f;

  if (soc < 0)
    soc = 0;
  if (soc > 100)
    soc = 100;
  return (uint8_t)(soc + 0.5f);
}

// ===== Charger control =====
bool NPM1300::enableCharger() { return writeRegister(npm1300::reg::BCHG_ENABLESET, 0x01); }
bool NPM1300::disableCharger() { return writeRegister(npm1300::reg::BCHG_ENABLECLR, 0x01); }

bool NPM1300::setChargeCurrent(uint16_t currentMa)
{
  if (currentMa < 32)
    currentMa = 32;
  if (currentMa > 800)
    currentMa = 800;

  uint8_t msb = currentMa / 2;
  uint8_t lsb = currentMa % 2;

  if (!writeRegister(npm1300::reg::BCHG_ISETMSB, msb))
    return false;
  if (!writeRegister(npm1300::reg::BCHG_ISETLSB, lsb))
    return false;
  return true;
}

bool NPM1300::setChargeVoltage(float voltage)
{
  uint8_t regValue = 0;

  if (voltage >= 3.50 && voltage < 3.525)
    regValue = 0;
  else if (voltage >= 3.525 && voltage < 3.575)
    regValue = 1;
  else if (voltage >= 3.575 && voltage < 3.625)
    regValue = 2;
  else if (voltage >= 3.625 && voltage < 3.75)
    regValue = 3;
  else if (voltage >= 3.75 && voltage < 4.025)
    regValue = 4;
  else if (voltage >= 4.025 && voltage < 4.075)
    regValue = 5;
  else if (voltage >= 4.075 && voltage < 4.125)
    regValue = 6;
  else if (voltage >= 4.125 && voltage < 4.175)
    regValue = 7;
  else if (voltage >= 4.175 && voltage < 4.225)
    regValue = 8;
  else if (voltage >= 4.225 && voltage < 4.275)
    regValue = 9;
  else if (voltage >= 4.275 && voltage < 4.325)
    regValue = 10;
  else if (voltage >= 4.325 && voltage < 4.375)
    regValue = 11;
  else if (voltage >= 4.375 && voltage < 4.425)
    regValue = 12;
  else if (voltage >= 4.425)
    regValue = 13;

  _vterm_mV = (uint16_t)(voltage * 1000.0f + 0.5f);
  return writeRegister(npm1300::reg::BCHG_VTERM, regValue);
}

bool NPM1300::setChargerIgnoreNTC(bool ignore)
{
  // bit1 = DISABLENTC (when set => ignore NTC)
  const uint8_t MASK = (1u << 1);
  return ignore
             ? writeRegister(npm1300::reg::BCHG_DISABLESET, MASK)
             : writeRegister(npm1300::reg::BCHG_DISABLECLR, MASK);
}

bool NPM1300::setChargerRechargeEnabled(bool enable)
{
  // bit0 = DISABLERECHARGE (when set => recharge disabled)
  const uint8_t MASK = (1u << 0);
  return enable
             ? writeRegister(npm1300::reg::BCHG_DISABLECLR, MASK)
             : writeRegister(npm1300::reg::BCHG_DISABLESET, MASK);
}

// decoded flags
bool NPM1300::isBatteryDetected() { return (readChgStatusRaw() & (1u << 0)) != 0; }
bool NPM1300::isChargeCompleted() { return (readChgStatusRaw() & (1u << 1)) != 0; }
bool NPM1300::isTrickleCharging() { return (readChgStatusRaw() & (1u << 2)) != 0; }
bool NPM1300::isConstantCurrentCharging() { return (readChgStatusRaw() & (1u << 3)) != 0; }
bool NPM1300::isConstantVoltageCharging() { return (readChgStatusRaw() & (1u << 4)) != 0; }
bool NPM1300::isRechargeNeeded() { return (readChgStatusRaw() & (1u << 5)) != 0; }
bool NPM1300::isDieTempHighPaused() { return (readChgStatusRaw() & (1u << 6)) != 0; }
bool NPM1300::isSupplementModeActive() { return (readChgStatusRaw() & (1u << 7)) != 0; }

// ===== GPIO =====
bool NPM1300::setGpioMode(uint8_t pin, uint8_t mode)
{
  if (pin > 4)
    return false;
  if (mode > 9)
    return false;
  return writeRegister(npm1300::reg::GPIO_MODE0 + pin, mode);
}

bool NPM1300::setGpioOutput(uint8_t pin, bool state)
{
  if (pin > 4)
    return false;
  uint8_t mode = state ? npm1300::GPIO_OUT_LOGIC1 : npm1300::GPIO_OUT_LOGIC0;
  return setGpioMode(pin, mode);
}

bool NPM1300::getGpioInput(uint8_t pin)
{
  if (pin > 4)
    return false;
  uint8_t status = 0;
  if (!readRegister(npm1300::reg::GPIO_STATUS, &status))
    return false;
  return (status & (1u << pin)) != 0;
}

bool NPM1300::setGpioPullUp(uint8_t pin, bool enable)
{
  if (pin > 4)
    return false;
  return writeRegister(npm1300::reg::GPIO_PUEN0 + pin, enable ? 1 : 0);
}

bool NPM1300::setGpioPullDown(uint8_t pin, bool enable)
{
  if (pin > 4)
    return false;
  return writeRegister(npm1300::reg::GPIO_PDEN0 + pin, enable ? 1 : 0);
}

// ===== LEDDRV =====
bool NPM1300::setLedMode(uint8_t led, LedMode mode)
{
  if (led > 2)
    return false;

  uint16_t reg =
      (led == 0) ? npm1300::reg::LEDDRV0MODESEL : (led == 1) ? npm1300::reg::LEDDRV1MODESEL
                                                             : npm1300::reg::LEDDRV2MODESEL;

  return writeRegister(reg, (uint8_t)mode);
}

bool NPM1300::ledOn(uint8_t led)
{
  if (led > 2)
    return false;

  uint16_t reg =
      (led == 0) ? npm1300::reg::LEDDRV0SET : (led == 1) ? npm1300::reg::LEDDRV1SET
                                                         : npm1300::reg::LEDDRV2SET;

  return writeRegister(reg, 1);
}

bool NPM1300::ledOff(uint8_t led)
{
  if (led > 2)
    return false;

  uint16_t reg =
      (led == 0) ? npm1300::reg::LEDDRV0CLR : (led == 1) ? npm1300::reg::LEDDRV1CLR
                                                         : npm1300::reg::LEDDRV2CLR;

  return writeRegister(reg, 1);
}

bool NPM1300::setAllLedsHostMode()
{
  return setLedMode(0, LED_MODE_HOST) &&
         setLedMode(1, LED_MODE_HOST) &&
         setLedMode(2, LED_MODE_HOST);
}

bool NPM1300::fuelGaugeBegin(float capacity_mAh, float initialSoc_percent)
{
  if (capacity_mAh <= 0.0f)
    return false;

  _fgCap_mAh = capacity_mAh;
  if (initialSoc_percent < 0.0f)
  {
    // If not provided, fall back to your voltage-based SOC once
    initialSoc_percent = float(getBatterySOC());
  }

  if (initialSoc_percent < 0.0f)
    initialSoc_percent = 0.0f;
  if (initialSoc_percent > 100.0f)
    initialSoc_percent = 100.0f;

  _fgRemain_mAh = _fgCap_mAh * (initialSoc_percent / 100.0f);
  _fgLastMs = millis();
  _fgAvgDischarge_mA = 0.0f;
  _fgAvgCharge_mA = 0.0f;
  _fgInit = true;
  return true;
}

void NPM1300::fuelGaugeReset(float capacity_mAh, float initialSoc_percent)
{
  _fgInit = false;
  fuelGaugeBegin(capacity_mAh, initialSoc_percent);
}

bool NPM1300::fuelGaugeUpdate()
{
  if (!_fgInit)
    return false;

  const uint32_t now = millis();
  uint32_t dt_ms = now - _fgLastMs;
  if (dt_ms == 0)
    return true;
  _fgLastMs = now;

  // Signed current from PMIC (mA). Positive=charging, Negative=discharging.
  const int16_t ibat_mA = getIBATmA();

  // convert ms -> hours
  const float dt_h = float(dt_ms) / 3600000.0f;

  // Coulomb integrate: mAh += mA * hours
  _fgRemain_mAh += float(ibat_mA) * dt_h;

  // clamp
  if (_fgRemain_mAh < 0.0f)
    _fgRemain_mAh = 0.0f;
  if (_fgRemain_mAh > _fgCap_mAh)
    _fgRemain_mAh = _fgCap_mAh;

  // update averages (EMA)
  const float alpha = 0.05f;
  if (ibat_mA < 0)
  {
    float d = float(-ibat_mA);
    _fgAvgDischarge_mA = (_fgAvgDischarge_mA <= 0.0f) ? d : (alpha * d + (1.0f - alpha) * _fgAvgDischarge_mA);
  }
  else if (ibat_mA > 0)
  {
    float c = float(ibat_mA);
    _fgAvgCharge_mA = (_fgAvgCharge_mA <= 0.0f) ? c : (alpha * c + (1.0f - alpha) * _fgAvgCharge_mA);
  }

  return true;
}

void NPM1300::fuelGaugeCalibrateFull()
{
  if (!_fgInit)
    return;
  _fgRemain_mAh = _fgCap_mAh;
}

void NPM1300::fuelGaugeCalibrateEmpty()
{
  if (!_fgInit)
    return;
  _fgRemain_mAh = 0.0f;
}

float NPM1300::fuelGaugeSoc() const
{
  if (!_fgInit || _fgCap_mAh <= 0.0f)
    return NAN;
  return (_fgRemain_mAh / _fgCap_mAh) * 100.0f;
}

float NPM1300::fuelGaugeRemaining_mAh() const { return _fgRemain_mAh; }
float NPM1300::fuelGaugeCapacity_mAh() const { return _fgCap_mAh; }

float NPM1300::fuelGaugeTTE_min() const
{
  if (!_fgInit)
    return NAN;
  if (_fgAvgDischarge_mA <= 1.0f)
    return NAN;
  // hours = mAh / mA
  return (_fgRemain_mAh / _fgAvgDischarge_mA) * 60.0f;
}

float NPM1300::fuelGaugeTTF_min() const
{
  if (!_fgInit)
    return NAN;
  if (_fgAvgCharge_mA <= 1.0f)
    return NAN;
  float missing = _fgCap_mAh - _fgRemain_mAh;
  if (missing <= 0.0f)
    return 0.0f;
  return (missing / _fgAvgCharge_mA) * 60.0f;
}

bool NPM1300::selectLdo1Mode(bool ldoMode)
{
  // LDSW1LDOSEL: bit0 = 1 => LDO, 0 => Load switch
  return writeRegister(npm1300::reg::LDSW1LDOSEL, ldoMode ? 1 : 0);
}

bool NPM1300::selectLdo2Mode(bool ldoMode)
{
  return writeRegister(npm1300::reg::LDSW2LDOSEL, ldoMode ? 1 : 0);
}

bool NPM1300::setLdo1Voltage(float voltage)
{
  // VOUTSEL uses same 1.0..3.3, 0.1V step encoding as your buck helper.
  uint8_t regValue = voltageToRegValue(voltage); // 0..23
  return writeRegister(npm1300::reg::LDSW1VOUTSEL, regValue);
}

bool NPM1300::setLdo2Voltage(float voltage)
{
  uint8_t regValue = voltageToRegValue(voltage);
  return writeRegister(npm1300::reg::LDSW2VOUTSEL, regValue);
}

bool NPM1300::enableLdo1()
{
  // TASKLDSW1SET: write 1 to enable output
  return writeRegister(npm1300::reg::TASKLDSW1SET, 1);
}

bool NPM1300::disableLdo1()
{
  return writeRegister(npm1300::reg::TASKLDSW1CLR, 1);
}

bool NPM1300::enableLdo2()
{
  return writeRegister(npm1300::reg::TASKLDSW2SET, 1);
}

bool NPM1300::disableLdo2()
{
  return writeRegister(npm1300::reg::TASKLDSW2CLR, 1);
}

uint8_t NPM1300::readVbusStatusRaw()
{
  uint8_t s = 0;
  readRegister(npm1300::reg::VBUSINSTATUS, &s);
  return s;
}

bool NPM1300::isVbusPresent()
{
  return (readVbusStatusRaw() & 0x01u) != 0; // bit0 = VBUSINPRESENT
}

// ===== Aliases =====
uint16_t NPM1300::readVBUS() { return getVbusVoltage(); }
uint16_t NPM1300::readVBAT() { return getVbatVoltage(); }
uint16_t NPM1300::readVSYS() { return getVsysVoltage(); }
uint8_t NPM1300::readSOC() { return getBatterySOC(); }

bool NPM1300::writeBuck1(float voltage) { return setBuck1Voltage(voltage); }
bool NPM1300::writeBuck2(float voltage) { return setBuck2Voltage(voltage); }
bool NPM1300::writeChargeCurrent(uint16_t mA) { return setChargeCurrent(mA); }
bool NPM1300::writeChargeVoltage(float voltage) { return setChargeVoltage(voltage); }

// ===== I2C helpers (16-bit reg) =====
bool NPM1300::writeRegister(uint16_t reg, uint8_t value)
{
  _i2c->beginTransmission(npm1300::I2C_ADDR);
  _i2c->write((uint8_t)(reg >> 8));
  _i2c->write((uint8_t)(reg & 0xFF));
  _i2c->write(value);
  return (_i2c->endTransmission() == 0);
}

bool NPM1300::readRegister(uint16_t reg, uint8_t *value)
{
  _i2c->beginTransmission(npm1300::I2C_ADDR);
  _i2c->write((uint8_t)(reg >> 8));
  _i2c->write((uint8_t)(reg & 0xFF));

  if (_i2c->endTransmission(false) != 0)
    return false;
  if (_i2c->requestFrom(npm1300::I2C_ADDR, (uint8_t)1) != 1)
    return false;

  *value = _i2c->read();
  return true;
}

bool NPM1300::readRegisters(uint16_t reg, uint8_t *buffer, uint8_t len)
{
  _i2c->beginTransmission(npm1300::I2C_ADDR);
  _i2c->write((uint8_t)(reg >> 8));
  _i2c->write((uint8_t)(reg & 0xFF));

  if (_i2c->endTransmission(false) != 0)
    return false;
  if (_i2c->requestFrom(npm1300::I2C_ADDR, len) != len)
    return false;

  for (uint8_t i = 0; i < len; i++)
    buffer[i] = _i2c->read();
  return true;
}

// ===== Conversion =====
uint8_t NPM1300::voltageToRegValue(float voltage)
{
  if (voltage < 1.0f)
    voltage = 1.0f;
  if (voltage > 3.3f)
    voltage = 3.3f;

  uint8_t regValue = (uint8_t)((voltage - 1.0f) / 0.1f + 0.5f);
  if (regValue > 23)
    regValue = 23;
  return regValue;
}