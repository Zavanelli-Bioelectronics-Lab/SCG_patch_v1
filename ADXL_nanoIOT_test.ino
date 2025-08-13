#include <SPI.h>

// ADXL355 Register Addresses
#define XDATA3 0x08
#define YDATA3 0x0B
#define ZDATA3 0x0E
#define RANGE  0x2C
#define POWER_CTL 0x2D

// Device values
#define RANGE_2G 0x01
#define MEASURE_MODE 0x06

// SPI operations
#define READ_BYTE  0x01
#define WRITE_BYTE 0x00

// Pin definitions
const int CHIP_SELECT_PIN = 10;

void setup() {
  Serial.begin(115200);
  SPI.begin();

  pinMode(CHIP_SELECT_PIN, OUTPUT);
  digitalWrite(CHIP_SELECT_PIN, HIGH); // Deselect sensor

  // Configure ADXL355
  writeRegister(RANGE, RANGE_2G);         // Set range to ±2g
  writeRegister(POWER_CTL, MEASURE_MODE); // Enable measurement mode

  delay(100);
  Serial.println("X\tY\tZ");  // Header for Serial Plotter
}

void loop() {
  int32_t x = readAxis(XDATA3);
  int32_t y = readAxis(YDATA3);
  int32_t z = readAxis(ZDATA3);

  Serial.print(x); Serial.print("\t");
  Serial.print(y); Serial.print("\t");
  Serial.println(z);

  delay(50);  // Smooth plotting
}

int32_t readAxis(uint8_t regMSB) {
  uint8_t msb = readRegister(regMSB);
  uint8_t mid = readRegister(regMSB - 1);
  uint8_t lsb = readRegister(regMSB - 2);

  uint32_t raw = ((uint32_t)msb << 16) | ((uint32_t)mid << 8) | lsb;
  raw >>= 4;  // Keep upper 20 bits

  // Sign extend to 32-bit signed integer
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
