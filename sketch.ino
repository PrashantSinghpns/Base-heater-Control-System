#include <DHT.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// ----- Pin definitions -----
#define DHT_PIN       18      // DHT22 data pin
#define DHT_TYPE      DHT22
#define RELAY_PIN     26      // Relay control (active LOW)
#define HEAT_LED_PIN  13      // LED indicator
#define IDLE_LED_PIN  25
#define BUZZER_PIN    27      // Buzzer

// Bar graph: 10 segments on pins
const int BAR_PINS[10] = {23, 19, 17, 16, 15, 14, 12, 5, 4, 2};

// ----- Temperature thresholds (°C) -----
const float MIN_TEMP            = 20.0;
const float TARGET_TEMP         = 28.0;
const float OVERHEAT_TEMP       = 45.0;
const float MIN_DISPLAY_TEMP    = -40.0;

// ----- Objects -----
DHT dht(DHT_PIN, DHT_TYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);

// ----- Global state -----
static float currentTemp = 0;
static String currentState = "";

// ----- Helpers -----
void setIndicators(bool heaterOn, bool heatLed, bool idleLed, bool buzzerOn) {
  digitalWrite(RELAY_PIN, heaterOn ? HIGH : LOW);  
  digitalWrite(HEAT_LED_PIN, heatLed);
  digitalWrite(IDLE_LED_PIN, idleLed);
  if (buzzerOn) tone(BUZZER_PIN, 500);
  else noTone(BUZZER_PIN);
}

// Since Wokwi cannot simulate a Real Heater, therefore,
// 10 segment LED BAR-Graph is used to depict Heater levels.
int heaterControlBar(float currentTemp) {
  // Bar graph logic
  int segments = map(currentTemp, MIN_DISPLAY_TEMP, TARGET_TEMP, 10, 0);
  segments = constrain(segments, 0, 10);

  for (int i = 0; i < 10; i++) {
    digitalWrite(BAR_PINS[i], i < segments ? HIGH : LOW);
  }

  return segments;
}

// ----- FreeRTOS Tasks -----

// Task 1: Read temperature every 2 seconds
void tempTask(void *pvParameters) {
    while (1) {
        float t = dht.readTemperature();
        // if temperature reading does not fail
        if (!isnan(t)) {
            currentTemp = t;
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Task 2: Control heater, LEDs, LCD, bar graph every 500ms
void controlTask(void *pvParameters) {
    float lastTemp = -999;
    String lastState = "";
    while (1) {
        // Determine state based on currentTemp
        if (currentTemp >= OVERHEAT_TEMP) {
            setIndicators(false, true, false, true);
            currentState = "OVERHEAT";
        } 
        else if (currentTemp < MIN_TEMP) {
            setIndicators(true, false, false, false);
            currentState = "HEATING";
        } 
        else if (currentTemp == TARGET_TEMP) {
            setIndicators(false, false, true, false);
            currentState = "TARGET_OK";
        } 
        else if (MIN_TEMP <= currentTemp && currentTemp < TARGET_TEMP) {
            setIndicators(true, false, true, false);
            currentState = "STABILIZING";
        } 
        else {
            setIndicators(false, false, true, false);
            currentState = "IDLE";
        }
      
        int segments = heaterControlBar(currentTemp);

        // Update LCD only when changed
        if (abs(currentTemp - lastTemp) > 0.1 || currentState != lastState) {
            lcd.clear();
            lcd.setCursor(0,0);
            lcd.print("Temp:"); lcd.print(currentTemp,1); lcd.print("C");

            lcd.setCursor(0,1);
            lcd.print("State:"); lcd.print(currentState);

            lastTemp = currentTemp;
            lastState = currentState;
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

// Task 3: Mock BLE advertising every 3 seconds via Serial,
// MOCK, since WOKWI does not have integrated BLE
void bleTask(void *pvParameters) {
    while (1) {
        Serial.print("MOCK BLE -> Temp="); Serial.print(currentTemp,1);
        Serial.print("C, State="); Serial.println(currentState);        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void setup() {
  // Serial
  Serial.begin(115200);
  delay(300);
  Serial.println("Heater Control Starting...");
  delay(100);

  // DHT sensor
  dht.begin();

  // LCD
  Wire.begin(21, 22);  // SDA, SCL
  lcd.init();
  lcd.backlight();

  // Output pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(HEAT_LED_PIN, OUTPUT);
  pinMode(IDLE_LED_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  
  digitalWrite(RELAY_PIN, HIGH);
  digitalWrite(HEAT_LED_PIN, LOW);
  digitalWrite(IDLE_LED_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  // Bar graph pins
  for (int i = 0; i < 10; i++) {
    pinMode(BAR_PINS[i], OUTPUT);
    digitalWrite(BAR_PINS[i], LOW);
  }

  // Create FreeRTOS tasks
  xTaskCreate(tempTask, "TempTask", 2048, NULL, 2, NULL);
  xTaskCreate(controlTask, "ControlTask", 4096, NULL, 2, NULL);
  xTaskCreate(bleTask, "BLETask", 2048, NULL, 1, NULL);
}

// Empty loop as tasks handle everything
void loop() {
  vTaskDelay(pdMS_TO_TICKS(100));
}

// ----- ARCHIVE CODE -----

// void loop() {
//   float temp = dht.readTemperature();
//   String state;
  
//   // Determine state and control outputs
//   if (temp >= OVERHEAT_TEMP) {
//     setIndicators(false, true, false, true);
//     state = "OVERHEAT";
//   } 
//   else if (temp < MIN_TEMP) {
//     setIndicators(true, true, false, false);
//     state = "HEATING";
//   } 
//   else if (temp == TARGET_TEMP) {
//     setIndicators(false, false, true, false);
//     state = "TARGET_OK";
//   } 
//   else if (MIN_TEMP <= temp && temp < TARGET_TEMP) {
//     setIndicators(true, true, true, false);
//     state = "STABILIZING";
//   } 
//   else {
//     setIndicators(false, false, true, false);
//     state = "IDLE";
//   }

//   int segments = heaterControl(temp);

//   // LCD display
//   lcd.clear();
//   lcd.setCursor(0, 0);
//   lcd.print("Temp:"); lcd.print(temp, 1); lcd.print("C");
//   lcd.setCursor(0, 1);
//   lcd.print("State:"); lcd.print(state);

//   // Serial log
//   Serial.print("Temp = "); Serial.print(temp);
//   Serial.print(", Status = "); Serial.print(state);
//   if (temp < TARGET_TEMP) {
//     Serial.print(", Heater Level = "); Serial.println(segments);
//   }
//   else {
//     Serial.println(", Heater OFF");
//   }

//   delay(300);
// }
