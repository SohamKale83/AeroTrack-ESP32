#include <Wire.h>
#include <ESP32BLECombo.h> 


ESP32BLECombo bleMouse;


const int MPU_ADDR = 0x68; 


const int PIN_LEFT_CLICK  = 12; 
const int PIN_RIGHT_CLICK = 14; 
const int PIN_WEB_BACK    = 27; 
const int PIN_WEB_FORWARD = 26; 

const int WHITE_LED  = 2;  
const int RED_LED    = 4;  
const int ANALOG_PIN = 32; 


const uint8_t CONFIG_MOUSE_LEFT    = 1;  
const uint8_t CONFIG_MOUSE_RIGHT   = 2;  

// State booleans to handle edge-triggering and avoid command flooding
bool isLeftPressed    = false;
bool isRightPressed   = false;
unsigned long lastClickTime = 0; 

// Non-blocking timer variables for background battery execution
unsigned long lastBatteryCheckTime = 0;
const unsigned long BATTERY_CHECK_INTERVAL = 1000; // 1 second intervals

// =========================================================================
// TUNING PARAMETERS: PRESERVED YOUR PREVIOUS WORKING VALUES
// =========================================================================
const float SENSITIVITY_X = 3.5;  
const float SENSITIVITY_Y = 4.0;  

const int GYRO_DEADZONE = 600;    

const float SMOOTHING_FACTOR = 0.35; 

// HARDCODED BIAS OFFSETS: Kept static to bypass startup calibration errors
const int16_t gyroY_cal = 550; 
const int16_t gyroZ_cal = 55;  

// SCROLL SPEED CONFIGURATION: Lines moved per loop cycle
const int SCROLL_SPEED = 1;
// =========================================================================

float smoothMoveX = 0;
float smoothMoveY = 0;
bool previouslyConnected = false;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); 

  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); 
  Wire.write(0);    
  Wire.endTransmission(true);

  // Initializing button inputs with internal pull-up configurations
  pinMode(PIN_LEFT_CLICK, INPUT_PULLUP);
  pinMode(PIN_RIGHT_CLICK, INPUT_PULLUP);
  pinMode(PIN_WEB_BACK, INPUT_PULLUP);
  pinMode(PIN_WEB_FORWARD, INPUT_PULLUP);

  // Initializing LED pins
  pinMode(WHITE_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  Serial.println("Starting BLE Combo Service...");
  
  ESP32BLEComboConfig cfg;
  cfg.deviceName = "ESP32 Air Pointer";
  cfg.manufacturer = "Custom_Maker"; 
  cfg.mode = ESP32BLEComboMode::MOUSE_ONLY;
  
  bleMouse.begin(cfg);
  Serial.println("Waiting for laptop pairing connection...");
}

void loop() {
  // Always keep the white LED lit to confirm execution status
  digitalWrite(WHITE_LED, HIGH);

  // --- NON-BLOCKING BATTERY MONITORING ---
  // Checks the battery metrics every 1000ms without freezing the loop
  if (millis() - lastBatteryCheckTime >= BATTERY_CHECK_INTERVAL) {
    lastBatteryCheckTime = millis();
    
    int analog_read = analogRead(ANALOG_PIN);
    float pin_voltage = (analog_read * 3.3) / 4095.0; 
    float battery_voltage = pin_voltage * 3.12; 

    Serial.print("Pin V: "); Serial.print(pin_voltage);
    Serial.print(" | Battery V: "); Serial.print(battery_voltage);
    Serial.println(" V");

    if (battery_voltage < 6.6) {
      digitalWrite(RED_LED, HIGH);  // Warning: Voltage low
    } else {
      digitalWrite(RED_LED, LOW);   // Battery healthy
    }
  }

  // --- AIR MOUSE ENGINE ---
  bool currentlyConnected = bleMouse.isConnected();

  if (currentlyConnected && !previouslyConnected) {
    Serial.println("\n=========================================");
    Serial.println("SUCCESS: ESP32 Air Pointer Connected!");
    Serial.println("=========================================");
    previouslyConnected = true;
  }
  else if (!currentlyConnected && previouslyConnected) {
    Serial.println("\n[!] Disconnected.");
    previouslyConnected = false;
  }

  if (currentlyConnected) {
    handleButtonInputs();
    
    // Suppress pointer micro-tremors immediately following click mechanics
    if (millis() - lastClickTime > 80) {
      handleMouseMovement();
    }
  }

  delay(10); // Quick refresh delay (~100Hz) ensures tracking fluidity
}

void handleMouseMovement() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x45); 
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 4, true); 

  int16_t rawGyroY = (int16_t)(Wire.read() << 8 | Wire.read());
  int16_t rawGyroZ = (int16_t)(Wire.read() << 8 | Wire.read());

  int16_t gyroY = rawGyroY - gyroY_cal;
  int16_t gyroZ = rawGyroZ - gyroZ_cal;

  float targetX = 0;
  float targetY = 0;

  if (abs(gyroZ) > GYRO_DEADZONE) {
    targetX = (-gyroZ * SENSITIVITY_X / 500.0); 
  }

  if (abs(gyroY) > GYRO_DEADZONE) {
    targetY = (gyroY * SENSITIVITY_Y / 500.0); 
  }

  smoothMoveX = (targetX * SMOOTHING_FACTOR) + (smoothMoveX * (1.0 - SMOOTHING_FACTOR));
  smoothMoveY = (targetY * SMOOTHING_FACTOR) + (smoothMoveY * (1.0 - SMOOTHING_FACTOR));

  int finalX = (int)smoothMoveX;
  int finalY = (int)smoothMoveY;

  if (finalX != 0 || finalY != 0) {
    bleMouse.mouseMove(finalX, finalY);
  }
}

void handleButtonInputs() {
  bool leftButtonState    = (digitalRead(PIN_LEFT_CLICK) == LOW);
  bool rightButtonState   = (digitalRead(PIN_RIGHT_CLICK) == LOW);
  bool scrollDownState    = (digitalRead(PIN_WEB_BACK) == LOW);
  bool scrollUpState      = (digitalRead(PIN_WEB_FORWARD) == LOW);

  // Top Left: Left Click
  if (leftButtonState && !isLeftPressed) {
    bleMouse.mousePress(CONFIG_MOUSE_LEFT);
    isLeftPressed = true;
    lastClickTime = millis(); 
  } else if (!leftButtonState && isLeftPressed) {
    bleMouse.mouseRelease(CONFIG_MOUSE_LEFT);
    isLeftPressed = false;
    lastClickTime = millis(); 
  }

  // Top Right: Right Click
  if (rightButtonState && !isRightPressed) {
    bleMouse.mousePress(CONFIG_MOUSE_RIGHT);
    isRightPressed = true;
    lastClickTime = millis(); 
  } else if (!rightButtonState && isRightPressed) {
    bleMouse.mouseRelease(CONFIG_MOUSE_RIGHT);
    isRightPressed = false;
    lastClickTime = millis(); 
  }

  // Bottom Left: Scroll Down
  if (scrollDownState) {
    bleMouse.mouseScroll(-SCROLL_SPEED);
    lastClickTime = millis(); 
  }

  // Bottom Right: Scroll Up
  if (scrollUpState) {
    bleMouse.mouseScroll(SCROLL_SPEED);
    lastClickTime = millis(); 
  }
}
  