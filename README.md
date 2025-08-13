Firmware files to support BLE communication and data collection with two Nano 33 IoT boards and an ADXL355 EVAL

Wiring Diagram

| ADXL355 Pin           | Arduino Pin | Notes                            |
| --------------------- | ----------- | -------------------------------- |
| **VDD**               | 3.3 V       | Powers the sensor core           |
| **VDDIO**             | 3.3 V       | Powers IO interface              |
| **GND**               | GND         | Ground                           |
| **MISO/SDA**          | D12 (MISO)  | Master In, Slave Out             |
| **MOSI/SDA**          | D11 (MOSI)  | Master Out, Slave In             |
| **CS/SCL**            | D10         | Chip Select (active low)         |
| **SCLK/VSSIO**        | D13 (SCK)   | SPI clock                        |
| **VSSIO**             | GND         | If separate pin for logic ground |
| **INT1/INT2/DRDY**    | (optional)  | Not needed for basic reads       |
| **V1pbana / V1p8dig** | —           | Leave unconnected                |
