#include <ArduinoBLE.h>
#include <SPI.h>

// BLE setup
BLEService customService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic dataCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLENotify, 9);  // 3 axes × 3 bytes

// ADXL355 registers
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
const int internalIntervalMicros = 2000;  // 500 Hz
unsigned long lastSampleTime = 0;

void setup() {
  SPI.begin();
  pinMode(CHIP_SELECT_PIN, OUTPUT);
  digitalWrite(CHIP_SELECT_PIN, HIGH);

  // Configure ADXL355
  writeRegister(RANGE, RANGE_2G);  // ±2 g
  writeRegister(POWER_CTL, MEASURE_MODE);
  configureFIRFilter(100);  // ~100 Hz LPF
  delay(100);

  // BLE
  if (!BLE.begin())
    while (1)
      ;

  BLE.setLocalName("Nano33IoT");
  BLE.setAdvertisedService(customService);
  customService.addCharacteristic(dataCharacteristic);
  BLE.addService(customService);
  BLE.advertise();
}

void loop() {
  BLEDevice central = BLE.central();
  if (central) {
    while (central.connected()) {
      unsigned long now = micros();
      if (now - lastSampleTime >= internalIntervalMicros) {
        lastSampleTime = now;

        // Read raw 24-bit register triplets
        uint8_t xraw[3] = { readRegister(XDATA3),
                            readRegister(XDATA3 + 1),
                            readRegister(XDATA3 + 2) };
        uint8_t yraw[3] = { readRegister(YDATA3),
                            readRegister(YDATA3 + 1),
                            readRegister(YDATA3 + 2) };
        uint8_t zraw[3] = { readRegister(ZDATA3),
                            readRegister(ZDATA3 + 1),
                            readRegister(ZDATA3 + 2) };

        uint8_t payload[9];
        payload[0] = xraw[0];
        payload[1] = xraw[1];
        payload[2] = xraw[2];
        payload[3] = yraw[0];
        payload[4] = yraw[1];
        payload[5] = yraw[2];
        payload[6] = zraw[0];
        payload[7] = zraw[1];
        payload[8] = zraw[2];

        dataCharacteristic.writeValue(payload, 9);
      }
    }
  }
}

// ---------------- ADXL355 Helpers ----------------
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

void configureFIRFilter(float cutoffHz) {
  uint8_t regVal = 2;  // ~100 Hz cutoff for 500 Hz ODR
  writeRegister(0x28, regVal);
}
