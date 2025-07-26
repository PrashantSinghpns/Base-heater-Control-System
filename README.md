# Base-heater-Control-System(ESP32 + DHT22)

##  Overview
This project simulates an automatic heater control system using an ESP32 and DHT22 sensor. It controls a relay, LEDs, buzzer, and LCD to manage heating states based on real-time temperature.

##  Features
- Temperature sensing (DHT22)
- Heater control using relay
- Visual indicators via LEDs and LCD
- Buzzer alert on overheat
- BLE status logging (mock in Wokwi)
- FreeRTOS-based multitasking
- Heater intensity via bar graph LEDs

##  Components
- ESP32
- DHT22 Sensor
- Relay Module
- 16x2 LCD (I2C)
- Buzzer
- LEDs (Heat, Idle, Overheat)
- 10-Segment LED Bar Graph

##  Logic Flow
1. Monitor temperature
2. Control heater via relay
3. Show state on LCD
4. Use bar graph to show heat levels
5. Alert with buzzer if temperature crosses safe limit

##  Wokwi Simulation
[ Click here to open project in Wokwi](https://wokwi.com/projects/437554051715852289)

##  Libraries Used
- `DHT`
- `Wire`
- `LiquidCrystal_I2C`
- `freertos/FreeRTOS.h`

##  Notes
- BLE is mocked via Serial due to Wokwi limitation
- Can be extended to use real BLE, MQTT, or Wi-Fi in real hardware
