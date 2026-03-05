#ifndef NPM1300_H
#define NPM1300_H

#include <Arduino.h>
#include <Wire.h>

// ==========================
// nPM1300 Register Map
// ==========================
namespace npm1300
{

  // I2C
  static constexpr uint8_t I2C_ADDR = 0x6B;

  // Base blocks (16-bit register map)

  static constexpr uint16_t VBUSIN_BASE = 0x0200;
  static constexpr uint16_t BCHARGER_BASE = 0x0300;
  static constexpr uint16_t BUCK_BASE = 0x0400;
  static constexpr uint16_t ADC_BASE = 0x0500;
  static constexpr uint16_t GPIO_BASE = 0x0600;
  static constexpr uint16_t LDSW_BASE = 0x0800;
  static constexpr uint16_t LEDDRV_BASE = 0x0A00;

  namespace reg
  {
    // ===== BUCK =====
    static constexpr uint16_t BUCK1_ENASET = BUCK_BASE + 0x00;
    static constexpr uint16_t BUCK1_ENACLR = BUCK_BASE + 0x01;
    static constexpr uint16_t BUCK2_ENASET = BUCK_BASE + 0x02;
    static constexpr uint16_t BUCK2_ENACLR = BUCK_BASE + 0x03;
    static constexpr uint16_t BUCK1_NORMVOUT = BUCK_BASE + 0x08;
    static constexpr uint16_t BUCK2_NORMVOUT = BUCK_BASE + 0x0A;
    static constexpr uint16_t BUCKSWCTRLSEL = BUCK_BASE + 0x0F;

    // ===== CHARGER =====
    static constexpr uint16_t BCHG_ENABLESET = BCHARGER_BASE + 0x04;
    static constexpr uint16_t BCHG_ENABLECLR = BCHARGER_BASE + 0x05;
    static constexpr uint16_t BCHG_DISABLESET = BCHARGER_BASE + 0x06;
    static constexpr uint16_t BCHG_DISABLECLR = BCHARGER_BASE + 0x07;
    static constexpr uint16_t BCHG_ISETMSB = BCHARGER_BASE + 0x08;
    static constexpr uint16_t BCHG_ISETLSB = BCHARGER_BASE + 0x09;
    static constexpr uint16_t BCHG_ISETDISCHARGEMSB = BCHARGER_BASE + 0x0A;
    static constexpr uint16_t BCHG_ISETDISCHARGELSB = BCHARGER_BASE + 0x0B;
    static constexpr uint16_t BCHG_VTERM = BCHARGER_BASE + 0x0C;
    static constexpr uint16_t BCHG_CHARGESTATUS = BCHARGER_BASE + 0x34;
    static constexpr uint16_t BCHG_ERRREASON = BCHARGER_BASE + 0x36;

    // ===== ADC TASKS / RESULTS =====
    static constexpr uint16_t TASK_VBAT_MEAS = ADC_BASE + 0x00;
    static constexpr uint16_t TASK_DIETEMP_MEAS = ADC_BASE + 0x02;
    static constexpr uint16_t TASK_VSYS_MEAS = ADC_BASE + 0x03;
    static constexpr uint16_t TASK_IBAT_MEAS = ADC_BASE + 0x06;
    static constexpr uint16_t TASK_VBUS_MEAS = ADC_BASE + 0x07;
    static constexpr uint16_t ADC_IBAT_STATUS = ADC_BASE + 0x10;
    static constexpr uint16_t ADC_VBAT_MSB = ADC_BASE + 0x11;
    static constexpr uint16_t ADC_VSYS_MSB = ADC_BASE + 0x14;
    static constexpr uint16_t ADC_DIETEMP_MSB = ADC_BASE + 0x13;
    static constexpr uint16_t ADC_GP0_LSBS = ADC_BASE + 0x15;

    static constexpr uint16_t ADC_VBAT0_MSB = ADC_BASE + 0x16;
    static constexpr uint16_t ADC_VBAT1_MSB = ADC_BASE + 0x17;
    static constexpr uint16_t ADC_VBAT2_MSB = ADC_BASE + 0x18;
    static constexpr uint16_t ADC_VBAT3_MSB = ADC_BASE + 0x19;
    static constexpr uint16_t ADC_GP1_LSBS = ADC_BASE + 0x1A;
    static constexpr uint16_t ADC_IBAT_EN = ADC_BASE + 0x24;

    // ===== GPIO =====
    static constexpr uint16_t GPIO_MODE0 = GPIO_BASE + 0x00;
    static constexpr uint16_t GPIO_PUEN0 = GPIO_BASE + 0x0A;
    static constexpr uint16_t GPIO_PDEN0 = GPIO_BASE + 0x0F;
    static constexpr uint16_t GPIO_STATUS = GPIO_BASE + 0x1E;

    // ===== LEDDRV =====
    static constexpr uint16_t LEDDRV0MODESEL = LEDDRV_BASE + 0x00;
    static constexpr uint16_t LEDDRV1MODESEL = LEDDRV_BASE + 0x01;
    static constexpr uint16_t LEDDRV2MODESEL = LEDDRV_BASE + 0x02;

    static constexpr uint16_t LEDDRV0SET = LEDDRV_BASE + 0x03;
    static constexpr uint16_t LEDDRV0CLR = LEDDRV_BASE + 0x04;
    static constexpr uint16_t LEDDRV1SET = LEDDRV_BASE + 0x05;
    static constexpr uint16_t LEDDRV1CLR = LEDDRV_BASE + 0x06;
    static constexpr uint16_t LEDDRV2SET = LEDDRV_BASE + 0x07;
    static constexpr uint16_t LEDDRV2CLR = LEDDRV_BASE + 0x08;

    // ===== LDSW / LDO =====
    static constexpr uint16_t TASKLDSW1SET = LDSW_BASE + 0x00;
    static constexpr uint16_t TASKLDSW1CLR = LDSW_BASE + 0x01;
    static constexpr uint16_t TASKLDSW2SET = LDSW_BASE + 0x02;
    static constexpr uint16_t TASKLDSW2CLR = LDSW_BASE + 0x03;

    static constexpr uint16_t LDSWSTATUS = LDSW_BASE + 0x04;

    static constexpr uint16_t LDSW1GPISEL = LDSW_BASE + 0x05;
    static constexpr uint16_t LDSW2GPISEL = LDSW_BASE + 0x06;

    static constexpr uint16_t LDSWCONFIG = LDSW_BASE + 0x07;

    static constexpr uint16_t LDSW1LDOSEL = LDSW_BASE + 0x08; // 0=LoadSwitch, 1=LDO
    static constexpr uint16_t LDSW2LDOSEL = LDSW_BASE + 0x09;

    static constexpr uint16_t LDSW1VOUTSEL = LDSW_BASE + 0x0C; // 0..23 => 1.0..3.3V
    static constexpr uint16_t LDSW2VOUTSEL = LDSW_BASE + 0x0D;

    static constexpr uint16_t VBUSINSTATUS = VBUSIN_BASE + 0x07;
  } // namespace reg

  // GPIO modes (your current mapping)
  static constexpr uint8_t GPIO_INPUT = 0;
  static constexpr uint8_t GPIO_OUT_LOGIC1 = 8;
  static constexpr uint8_t GPIO_OUT_LOGIC0 = 9;

} // namespace npm1300

// ==========================
// Driver Class
// ==========================
class NPM1300
{
public:
  explicit NPM1300(TwoWire &wirePort = Wire);

  bool begin();
  bool isConnected();

  // BUCK
  bool enableBuck1();
  bool enableBuck2();
  bool disableBuck1();
  bool disableBuck2();
  bool setBuck1Voltage(float voltage);
  bool setBuck2Voltage(float voltage);

  // Voltages (mV)
  uint16_t getVbusVoltage();
  uint16_t getVbatVoltage();
  uint16_t getVsysVoltage();

  // SOC
  uint8_t getBatterySOC();

  // Charging state
  bool isCharging();    // TRICKLE/CC/CV
  bool isBatteryFull(); // COMPLETED

  // Charger control
  bool enableCharger();
  bool disableCharger();
  bool setChargeCurrent(uint16_t currentMa);
  bool setChargeVoltage(float voltage);

  bool setChargerIgnoreNTC(bool ignore);
  bool setChargerRechargeEnabled(bool enable);

  // Charger debug
  uint8_t readChgStatusRaw();
  uint8_t readChgErrReasonRaw();

  // Decoded flags from BCHG_CHARGESTATUS
  bool isBatteryDetected();
  bool isChargeCompleted();
  bool isTrickleCharging();
  bool isConstantCurrentCharging();
  bool isConstantVoltageCharging();
  bool isRechargeNeeded();
  bool isDieTempHighPaused();
  bool isSupplementModeActive();

  // GPIO
  bool setGpioMode(uint8_t pin, uint8_t mode);
  bool setGpioOutput(uint8_t pin, bool state);
  bool getGpioInput(uint8_t pin);
  bool setGpioPullUp(uint8_t pin, bool enable);
  bool setGpioPullDown(uint8_t pin, bool enable);

  // ===== LDSW / LDO =====
  bool setLdo1Voltage(float voltage);
  bool setLdo2Voltage(float voltage);

  bool selectLdo1Mode(bool ldoMode); // true=LDO, false=LoadSwitch
  bool selectLdo2Mode(bool ldoMode);

  bool enableLdo1();
  bool disableLdo1();
  bool enableLdo2();
  bool disableLdo2();

  // Aliases
  uint16_t readVBUS();
  uint16_t readVBAT();
  uint16_t readVSYS();
  uint8_t readSOC();

  bool enableAutoIBAT(bool en);
  uint8_t readIBATStatusRaw();
  int16_t getIBATmA(); // will be TODO until we know result regs + scaling

  uint16_t getDieTempRaw();
  float getDieTempC(); // we'll keep conversion TODO until we confirm formula

  bool writeBuck1(float voltage);
  bool writeBuck2(float voltage);
  bool writeChargeCurrent(uint16_t mA);
  bool writeChargeVoltage(float voltage);

  bool isVbusPresent();
  uint8_t readVbusStatusRaw();

  // LEDDRV
  enum LedMode : uint8_t
  {
    LED_MODE_ERROR = 0,
    LED_MODE_CHARGING = 1,
    LED_MODE_HOST = 2
  };

  bool setLedMode(uint8_t led, LedMode mode); // led=0..2
  bool ledOn(uint8_t led);
  bool ledOff(uint8_t led);
  bool setAllLedsHostMode();

  // ===== Fuel Gauge (simple coulomb counter) =====
  bool fuelGaugeBegin(float capacity_mAh, float initialSoc_percent = -1.0f);
  void fuelGaugeReset(float capacity_mAh, float initialSoc_percent);
  bool fuelGaugeUpdate(); // call periodically (ex: 1s)

  void fuelGaugeCalibrateFull();  // call when charge completed
  void fuelGaugeCalibrateEmpty(); // call when you know battery is empty

  float fuelGaugeSoc() const; // 0..100
  float fuelGaugeRemaining_mAh() const;
  float fuelGaugeCapacity_mAh() const;

  // Optional: rough time estimates
  float fuelGaugeTTE_min() const; // time to empty (minutes), discharge only
  float fuelGaugeTTF_min() const; // time to full  (minutes), charge only

  // Useful getters for app layer
  uint16_t getVterm_mV() const { return _vterm_mV; }
  void setSocEmpty_mV(uint16_t mv) { _socEmpty_mV = mv; }

private:
  TwoWire *_i2c;

  // For SOC estimation
  uint16_t _vterm_mV = 4200;    // updated when you call setChargeVoltage()
  uint16_t _socEmpty_mV = 3300; // tweak for your battery/load

  // Fuel gauge state
  bool _fgInit = false;
  float _fgCap_mAh = 0.0f;
  float _fgRemain_mAh = 0.0f;
  uint32_t _fgLastMs = 0;

  // For tte/ttf smoothing
  float _fgAvgDischarge_mA = 0.0f;
  float _fgAvgCharge_mA = 0.0f;

  bool writeRegister(uint16_t reg, uint8_t value);
  bool readRegister(uint16_t reg, uint8_t *value);
  bool readRegisters(uint16_t reg, uint8_t *buffer, uint8_t len);

  uint8_t voltageToRegValue(float voltage);
};

#endif // NPM1300_H