#include <ArduinoBLE.h>

// BLE service and characteristic using BLENotify
BLEService customService("19B10000-E8F2-537E-4F6C-D104768A1214");
BLECharacteristic dataCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify, 20);

int counter = 0;

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

  dataCharacteristic.writeValue("Waiting...");
  BLE.advertise();

  Serial.println("✅ BLE device ready and advertising.");
}

void loop() {
  delay(500);  // Stabilize BLE stack

  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("🔗 Connected to central: ");
    Serial.println(central.address());

    delay(10000);  // Give client time to subscribe (even if unused)

    while (central.connected()) {
      String message = "Count: " + String(counter++);
      dataCharacteristic.writeValue(message.c_str());
      Serial.println("📤 Sent: " + message);
      delay(2000);  // Send every 2 seconds
    }

    Serial.print("🔌 Disconnected from central: ");
    Serial.println(central.address());
  }
}
