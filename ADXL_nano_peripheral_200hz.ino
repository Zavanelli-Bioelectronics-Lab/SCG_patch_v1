#include <ArduinoBLE.h>
#include <SPI.h>

// BLE setup
BLEService customService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic dataCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLENotify, 9); // 3 bytes × 3 axes
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

unsigned long lastSendTime = 0;
const int sendIntervalMicros = 5000; // 100 Hz = 10,000 µs

void setup() {
 // Serial.begin(115200);
 // while (!Serial);

  SPI.begin();
  pinMode(CHIP_SELECT_PIN, OUTPUT);
  digitalWrite(CHIP_SELECT_PIN, HIGH);
  writeRegister(RANGE, RANGE_2G);
  writeRegister(POWER_CTL, MEASURE_MODE);
  delay(100);

  if (!BLE.begin()) {
   // Serial.println("❌ BLE init failed!");
    while (1);
  }
  BLE.setLocalName("Nano33IoT");
  BLE.setAdvertisedService(customService);
  customService.addCharacteristic(dataCharacteristic);
  BLE.addService(customService);
  BLE.advertise();
 // Serial.println("✅ Peripheral advertising.");
}

void loop() {
  BLEDevice central = BLE.central();
  if (central) {
   // Serial.print("🔗 Connected to central: ");
   // Serial.println(central.address());
    while (central.connected()) {
      unsigned long now = micros();
      if (now - lastSendTime >= sendIntervalMicros) {
        lastSendTime = now;

        int32_t x = readAxis(XDATA3);
        int32_t y = readAxis(YDATA3);
        int32_t z = readAxis(ZDATA3);

        uint8_t payload[9];
        payload[0] = (x >> 16) & 0xFF;
        payload[1] = (x >> 8) & 0xFF;
        payload[2] = x & 0xFF;
        payload[3] = (y >> 16) & 0xFF;
        payload[4] = (y >> 8) & 0xFF;
        payload[5] = y & 0xFF;
        payload[6] = (z >> 16) & 0xFF;
        payload[7] = (z >> 8) & 0xFF;
        payload[8] = z & 0xFF;

        dataCharacteristic.writeValue(payload, 9);
      }
    }
    Serial.println("🔌 Disconnected.");
  }
}

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
