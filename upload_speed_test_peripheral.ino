#include <ArduinoBLE.h>

BLEService customService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic dataCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLENotify, 3);

unsigned long lastSendTime = 0;
const int sendIntervalMicros = 2000; // 500 Hz = 2000 µs

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("❌ BLE init failed!");
    while (1);
  }

  BLE.setLocalName("Nano33IoT");
  BLE.setAdvertisedService(customService);
  customService.addCharacteristic(dataCharacteristic);
  BLE.addService(customService);

  BLE.advertise();
  Serial.println("✅ Peripheral advertising.");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("🔗 Connected to central: ");
    Serial.println(central.address());

    while (central.connected()) {
      unsigned long now = micros();
      if (now - lastSendTime >= sendIntervalMicros) {
        lastSendTime = now;

        // Simulate 20-bit data (3 bytes)
        uint8_t dummyData[3];
        dummyData[0] = random(0, 256);
        dummyData[1] = random(0, 256);
        dummyData[2] = random(0, 256);

        dataCharacteristic.writeValue(dummyData, 3);
      }
    }

    Serial.println("🔌 Disconnected.");
  }
}
