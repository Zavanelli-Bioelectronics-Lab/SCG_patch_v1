#include <ArduinoBLE.h>

const char* targetDeviceName = "Nano33IoT";
const char* targetServiceUUID = "19B10000-E8F2-537E-4F6C-D104768A1214";
const char* targetCharUUID    = "19B10001-E8F2-537E-4F6C-D104768A1214";

BLEDevice peripheral;
BLECharacteristic dataCharacteristic;
bool subscribed = false;

void setup() {
  Serial.begin(9600);
  while (!Serial);

  if (!BLE.begin()) {
    Serial.println("❌ BLE init failed!");
    while (1);
  }

  Serial.println("🔍 Scanning for BLE devices...");
  BLE.scan();
}

void loop() {
  if (!subscribed) {
    BLEDevice device = BLE.available();

    if (device && device.localName() == targetDeviceName) {
      Serial.println("🎯 Target device matched. Attempting connection...");
      BLE.stopScan();

      if (device.connect()) {
        Serial.println("✅ Connected!");
        delay(1500);  // Stabilize BLE stack

        bool success = device.discoverAttributes();
        if (!success) {
          Serial.println("⚠️ First discovery failed. Retrying...");
          delay(1000);
          success = device.discoverAttributes();
        }

        if (success) {
          Serial.println("🔍 Services discovered:");
          int serviceCount = device.serviceCount();
          for (int i = 0; i < serviceCount; i++) {
            BLEService service = device.service(i);
            Serial.print("🧪 Service UUID: ");
            Serial.println(service.uuid());

            
        if (String(service.uuid()).equalsIgnoreCase(targetServiceUUID)){
              Serial.println("🎯 Target service FOUND!");

              BLECharacteristic characteristic = service.characteristic(targetCharUUID);
              if (characteristic) {
                Serial.println("📡 Found target characteristic.");

                if (characteristic.canSubscribe()) {
                  characteristic.subscribe();
                  Serial.println("✅ Subscribed to notifications.");
                  dataCharacteristic = characteristic;
                  peripheral = device;
                  subscribed = true;
                } else {
                  Serial.println("⚠️ Characteristic does not support notifications.");
                  device.disconnect();
                }
              } else {
                Serial.println("❌ Target characteristic not found.");
                device.disconnect();
              }
              break; // Stop scanning services once found
            }
          }

          if (!subscribed) {
            Serial.println("❌ Target service not found.");
            device.disconnect();
          }
        } else {
          Serial.println("❌ Failed to discover attributes.");
          device.disconnect();
        }

        if (!subscribed) {
          BLE.scan(); // Restart scan
        }
      } else {
        Serial.println("❌ Connection failed.");
        BLE.scan(); // Restart scan
      }
    }
  } else {
    if (dataCharacteristic.valueUpdated()) {
      int len = dataCharacteristic.valueLength();
      const uint8_t* rawData = dataCharacteristic.value();

      char buffer[21]; // 20 bytes + null terminator
      memcpy(buffer, rawData, len);
      buffer[len] = '\0';

      String received = String(buffer);
      Serial.println("📥 Received: " + received);
    }

    if (!peripheral.connected()) {
      Serial.println("🔌 Disconnected. Restarting scan...");
      subscribed = false;
      peripheral = BLEDevice();
      BLE.scan();
    }
  }

  delay(100);
}
