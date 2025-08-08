#include <ArduinoBLE.h>

const char* serviceUUID = "19B10000-E8F2-537E-4F6C-D104768A1214";
const char* charUUID    = "19B10001-E8F2-537E-4F6C-D104768A1214";

BLEDevice peripheral;
BLECharacteristic dataCharacteristic;
bool subscribed = false;

void setup() {
  Serial.begin(115200); // Use higher baud rate for faster logging
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("❌ BLE init failed!");
    while (1);
  }

  Serial.println("🔍 Scanning...");
  BLE.scan();
}

void loop() {
  if (!subscribed) {
    BLEDevice device = BLE.available();

    if (device && device.localName() == "Nano33IoT") {
      BLE.stopScan();

      if (device.connect()) {
        delay(1000); // Stabilize

        if (device.discoverAttributes()) {
          BLEService service = device.service(serviceUUID);
          if (String(service.uuid()).equalsIgnoreCase(serviceUUID)) {
            dataCharacteristic = service.characteristic(charUUID);
            if (dataCharacteristic.canSubscribe()) {
              dataCharacteristic.subscribe();
              peripheral = device;
              subscribed = true;
              Serial.println("✅ Subscribed.");
            }
          }
        }
      }
    }
  } else {
    if (dataCharacteristic.valueUpdated()) {
      const uint8_t* data = dataCharacteristic.value();
      unsigned long timestamp = micros();

      Serial.print("📥 ");
      Serial.print(timestamp);
      Serial.print(" µs - Data: ");
      for (int i = 0; i < dataCharacteristic.valueLength(); i++) {
        Serial.print(data[i], HEX);
        Serial.print(" ");
      }
      Serial.println();
    }

    if (!peripheral.connected()) {
      Serial.println("🔌 Disconnected.");
      subscribed = false;
      BLE.scan();
    }
  }

  delay(1); // Keep loop responsive
}
