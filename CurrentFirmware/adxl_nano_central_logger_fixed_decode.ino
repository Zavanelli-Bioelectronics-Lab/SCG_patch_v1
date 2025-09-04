#include <ArduinoBLE.h>

const char* serviceUUID = "19B10000-E8F2-537E-4F6C-D104768A1214";
const char* charUUID    = "19B10001-E8F2-537E-4F6C-D104768A1214";

BLEDevice peripheral;
BLECharacteristic dataCharacteristic;
bool subscribed = false;

void setup() {
  Serial.begin(230400);
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
        delay(1000);
        if (device.discoverAttributes()) {
          BLEService service = device.service(serviceUUID);
          if (String(service.uuid()).equalsIgnoreCase(serviceUUID)) {
            dataCharacteristic = service.characteristic(charUUID);
            if (dataCharacteristic.canSubscribe()) {
              dataCharacteristic.subscribe();
              peripheral = device;
              subscribed = true;
              Serial.println("✅ Subscribed to ADXL data.");
            }
          }
        }
      }
    }
  } else {
    if (dataCharacteristic.valueUpdated()) {
      const uint8_t* data = dataCharacteristic.value();
      if (dataCharacteristic.valueLength() == 9) {
        // Decode X, Y, Z (20-bit signed)
        int32_t x = ((int32_t)data[0] << 16) |
                    ((int32_t)data[1] << 8)  |
                    data[2];

        int32_t y = ((int32_t)data[3] << 16) |
                    ((int32_t)data[4] << 8)  |
                    data[5];

        int32_t z = ((int32_t)data[6] << 16) |
                    ((int32_t)data[7] << 8)  |
                    data[8];

        // Right-shift 4 bits to drop unused nibble
        x >>= 4;  
        y >>= 4;  
        z >>= 4;  

        // Sign extend 20-bit values
        if (x & 0x80000) x |= 0xFFF00000;
        if (y & 0x80000) y |= 0xFFF00000;
        if (z & 0x80000) z |= 0xFFF00000;

        Serial.print(x); Serial.print(",");
        Serial.print(y); Serial.print(",");
        Serial.println(z);
      }
    }

    if (!peripheral.connected()) {
      Serial.println("🔌 Disconnected.");
      subscribed = false;
      BLE.scan();
    }
  }
}
