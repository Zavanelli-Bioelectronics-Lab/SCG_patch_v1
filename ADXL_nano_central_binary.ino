#include <ArduinoBLE.h>

const char* serviceUUID = "19B10000-E8F2-537E-4F6C-D104768A1214";
const char* charUUID    = "19B10001-E8F2-537E-4F6C-D104768A1214";

BLEDevice peripheral;
BLECharacteristic dataCharacteristic;

bool subscribed = false;

void setup() {
  Serial.begin(460800); // high-speed serial for binary logging
  while (!Serial);

  if (!BLE.begin()) {
    while (1); // BLE init failed
  }

  BLE.scan();
}

void loop() {
  if (!subscribed) {
    BLEDevice device = BLE.available();
    if (device) {
      if (device.localName() == "Nano33IoT") {
        BLE.stopScan();

        if (device.connect()) {
          delay(500); // allow time for discovery
          if (device.discoverAttributes()) {
            BLEService service = device.service(serviceUUID);
            if (service) {
              dataCharacteristic = service.characteristic(charUUID);
              if (dataCharacteristic.canSubscribe()) {
                dataCharacteristic.subscribe();
                peripheral = device;
                subscribed = true;
              }
            }
          }
        }
      }
    }
  } else {
    // Receive BLE notifications
    if (dataCharacteristic.valueUpdated()) {
      const uint8_t* data = dataCharacteristic.value();
      Serial.write(data, 9); // send raw 9-byte packet to PC
    }

    // Reconnect if disconnected
    if (!peripheral.connected()) {
      subscribed = false;
      BLE.scan();
    }
  }
}
