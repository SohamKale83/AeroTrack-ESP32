#include <Wire.h>
#include <ESP32BLECombo.h> // Using kokodev's optimized library

// Instantiate the BLE Combo object
ESP32BLECombo bleMouse;

// Hardware Configurations
const int MPU_ADDR = 0x68; // Standard I2C address for MPU6050/6500

// Button GPIO Pin Definitions (Kept only Left and Right Click)
const int PIN_LEFT_CLICK = 12;
const int PIN_RIGHT_CLICK = 14;

// Manually mapping explicit HID Mouse Codes to match library functions
const uint8_t CONFIG_MOUSE_LEFT  = 0x01;
const uint8_t CONFIG_MOUSE_RIGHT = 0x02;

// Track the state of the mouse buttons to prevent spamming press/release
bool isLeftPressed  = false;
bool isRightPressed = false;

// Adjustable Sensitivity Variables (Tweak these to alter cursor speed)
const float SENSITIVITY_X = 0.35; // Maps Gyro X (Roll) to Screen X
const float SENSITIVITY_Y = 0.45; // Maps Gyro Y (Pitch) to Screen Y
const int GYRO_DEADZONE = 180;    // Filter value to absorb your ~2500 resting noise

// Calibration Variables
int16_t gyroX_cal = 0;
int16_t gyroY_cal = 0;

// Tracking connection transitions for the serial print notification
bool previouslyConnected = false;

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // Initialize I2C (SDA = 21, SCL = 22)

  // 1. Initialize MPU6050/6500 Power Management Register
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x6B); // PWR_MGMT_1 register
  Wire.write(0);    // Set to 0 to wake up the IMU sensor
  Wire.endTransmission(true);

  // 2. Configure Button Pins using Internal Pullups
  pinMode(PIN_LEFT_CLICK, INPUT_PULLUP);
  pinMode(PIN_RIGHT_CLICK, INPUT_PULLUP);

  // 3. Configure and Start Bluetooth Service
  Serial.println("Starting BLE Service...");
  ESP32BLEComboConfig cfg;
  cfg.deviceName = "ESP32 Air Pointer";
  cfg.manufacturer = "Custom_Maker"; 
  
  // Set mode to mouse only to optimize performance and memory footprint
  cfg.mode = ESP32BLEComboMode::MOUSE_ONLY; 
  
  bleMouse.begin(cfg);
  Serial.println("Waiting for laptop pairing connection...");

  // 4. Run Sensor Gyro Calibration (Keep setup completely flat and still!)
  calibrateGyro();
}

void loop() {
  bool currentlyConnected = bleMouse.isConnected();

  // Crisp connection feedback to serial monitor on state change
  if (currentlyConnected && !previouslyConnected) {
    Serial.println("\n=========================================");
    Serial.println("SUCCESS: ESP32 Air Pointer Connected via Bluetooth!");
    Serial.println("=========================================");
    previouslyConnected = true;
  }
  else if (!currentlyConnected && previouslyConnected) {
    Serial.println("\n[!] Disconnected. Re-advertising Bluetooth...");
    previouslyConnected = false;
  }

  // Only read sensors and process inputs if Bluetooth is active
  if (currentlyConnected) {
    handleMouseMovement();
    handleButtonInputs();
  }

  delay(10); // Polling rate delay (~100Hz refresh rate)
}

// Automatically calculates resting baseline offsets to absorb your 2500 baseline noise
void calibrateGyro() {
  long sumX = 0;
  long sumY = 0;
  const int samples = 200;
  
  Serial.print("Calibrating IMU sensor (Keep still)...");
  for (int i = 0; i < samples; i++) {
    Wire.beginTransmission(MPU_ADDR);
    Wire.write(0x43); // Start reading from Gyro X register
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_ADDR, 4, true); // Request 4 bytes (Gyro X & Gyro Y)

    int16_t rawX = (Wire.read() << 8) | Wire.read();
    int16_t rawY = (Wire.read() << 8) | Wire.read();

    sumX += rawX;
    sumY += rawY;
    delay(5);
  }
  gyroX_cal = sumX / samples;
  gyroY_cal = sumY / samples;
  Serial.println(" Done Calibration!");
}

// Bypasses Gyro Z; maps Gyro X (Roll) and Gyro Y (Pitch) to cursor coordinates
void handleMouseMovement() {
  Wire.beginTransmission(MPU_ADDR);
  Wire.write(0x43); // Start reading from Gyro X register
  Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 4, true); // Grab 4 bytes

  int16_t rawGyroX = (Wire.read() << 8) | Wire.read();
  int16_t rawGyroY = (Wire.read() << 8) | Wire.read();

  // Subtract calibration bias (this turns your 2500 readings into 0 at rest)
  int16_t gyroX = rawGyroX - gyroX_cal;
  int16_t gyroY = rawGyroY - gyroY_cal;

  int moveX = 0;
  int moveY = 0;

  // Horizontal movement (via Roll Axis)
  if (abs(gyroX) > GYRO_DEADZONE) {
    moveX = (int)(gyroX * SENSITIVITY_X / 500); 
  }

  // Vertical movement (via Pitch Axis)
  if (abs(gyroY) > GYRO_DEADZONE) {
    moveY = (int)(gyroY * SENSITIVITY_Y / 500);
  }

  // Send move report if movement occurs
  if (moveX != 0 || moveY != 0) {
    bleMouse.mouseMove(moveX, moveY);
  }
}

// Polls Left/Right click hardware pins and transmits click commands
void handleButtonInputs() {
  // Read inputs (LOW means the button is actively pressed due to INPUT_PULLUP)
  bool leftButtonState  = (digitalRead(PIN_LEFT_CLICK) == LOW);
  bool rightButtonState = (digitalRead(PIN_RIGHT_CLICK) == LOW);

  // Managing Left Click using custom state boolean transitions
  if (leftButtonState && !isLeftPressed) {
    bleMouse.mousePress(CONFIG_MOUSE_LEFT);
    isLeftPressed = true;
  } else if (!leftButtonState && isLeftPressed) {
    bleMouse.mouseRelease(CONFIG_MOUSE_LEFT);
    isLeftPressed = false;
  }

  // Managing Right Click using custom state boolean transitions
  if (rightButtonState && !isRightPressed) {
    bleMouse.mousePress(CONFIG_MOUSE_RIGHT);
    isRightPressed = true;
  } else if (!rightButtonState && isRightPressed) {
    bleMouse.mouseRelease(CONFIG_MOUSE_RIGHT);
    isRightPressed = false;
  }
}
