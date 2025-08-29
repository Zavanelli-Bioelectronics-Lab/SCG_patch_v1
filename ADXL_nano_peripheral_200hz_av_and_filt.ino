#include <ArduinoBLE.h>
#include <SPI.h>

// BLE setup
BLEService customService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic dataCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLENotify, 9); // 3 axes × 3 bytes

// ADXL355 setup
#define XDATA3 0x08
#define YDATA3 0x0B
#define ZDATA3 0x0E
#define RANGE 0x2C
#define POWER_CTL 0x2D
#define RANGE_2G 0x01
#define MEASURE_MODE 0x06
#define READ_BYTE 0x01
#define WRITE_BYTE 0x00
const int CHIP_SELECT_PIN = 10;

// Timing
unsigned long lastBLETime = 0;
const int bleIntervalMicros = 5000; // 200 Hz BLE
const int internalIntervalMicros = 2000; // 500 Hz sampling

// Averaging
const int samplesPerBLE = bleIntervalMicros / internalIntervalMicros;
int sampleCount = 0;
int64_t xSum = 0, ySum = 0, zSum = 0;

void setup() {
  SPI.begin();
  pinMode(CHIP_SELECT_PIN, OUTPUT);
  digitalWrite(CHIP_SELECT_PIN, HIGH);

  // ADXL355 config
  writeRegister(RANGE, RANGE_2G);        // max sensitivity
  writeRegister(POWER_CTL, MEASURE_MODE);
  configureFIRFilter(100);               // low-pass 100 Hz cutoff
  delay(100);

  if (!BLE.begin()) while(1);

  BLE.setLocalName("Nano33IoT");
  BLE.setAdvertisedService(customService);
  customService.addCharacteristic(dataCharacteristic);
  BLE.addService(customService);
  BLE.advertise();
}

// ------------------ Loop ------------------
void loop() {
  BLEDevice central = BLE.central();
  if (central) {
    while (central.connected()) {
      unsigned long now = micros();
      static unsigned long lastSampleTime = 0;

      // Sample at internal rate (500 Hz)
      if (now - lastSampleTime >= internalIntervalMicros) {
        lastSampleTime = now;

        int32_t x = readAxis(XDATA3);
        int32_t y = readAxis(YDATA3);
        int32_t z = readAxis(ZDATA3);

        xSum += x;
        ySum += y;
        zSum += z;
        sampleCount++;

        // Send averaged value at BLE rate (200 Hz)
        if (sampleCount >= samplesPerBLE) {
          int32_t xAvg = xSum / sampleCount;
          int32_t yAvg = ySum / sampleCount;
          int32_t zAvg = zSum / sampleCount;

          uint8_t payload[9];
          payload[0] = (xAvg >> 16) & 0xFF;
          payload[1] = (xAvg >> 8) & 0xFF;
          payload[2] = xAvg & 0xFF;
          payload[3] = (yAvg >> 16) & 0xFF;
          payload[4] = (yAvg >> 8) & 0xFF;
          payload[5] = yAvg & 0xFF;
          payload[6] = (zAvg >> 16) & 0xFF;
          payload[7] = (zAvg >> 8) & 0xFF;
          payload[8] = zAvg & 0xFF;

          dataCharacteristic.writeValue(payload, 9);

          // Reset sums
          sampleCount = 0;
          xSum = 0; ySum = 0; zSum = 0;
        }
      }
    }
  }
}

// ------------------ ADXL355 Functions ------------------
int32_t readAxis(uint8_t regMSB) {
  uint8_t msb = readRegister(regMSB);
  uint8_t mid = readRegister(regMSB - 1);
  uint8_t lsb = readRegister(regMSB - 2);
  uint32_t raw = ((uint32_t)msb << 16) | ((uint32_t)mid << 8) | lsb;
  raw >>= 4;
  if (raw & 0x80000) raw |= 0xFFF00000;
  return (int32_t)raw;
}

void writeRegister(uint8_t reg, uint8_t value) {
  uint8_t command = (reg << 1) | WRITE_BYTE;
  digitalWrite(CHIP_SELECT_PIN, LOW);
  SPI.transfer(command);
  SPI.transfer(value);
  digitalWrite(CHIP_SELECT_PIN, HIGH);
}

uint8_t readRegister(uint8_t reg) {
  uint8_t command = (reg << 1) | READ_BYTE;
  digitalWrite(CHIP_SELECT_PIN, LOW);
  SPI.transfer(command);
  uint8_t result = SPI.transfer(0x00);
  digitalWrite(CHIP_SELECT_PIN, HIGH);
  return result;
}

// ------------------ FIR Filter Config ------------------
void configureFIRFilter(float cutoffHz) {
  // ADXL355 filter: filter register 0x28 = low-pass filter settings
  // Filter register values: cutoff = ODR / (2^(value+1))
  // For 500 Hz ODR, choose value ~2 for ~100 Hz cutoff
  uint8_t regVal = 2;
  writeRegister(0x28, regVal);
}
