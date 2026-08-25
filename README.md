# 🖱️ AeroTrack-ESP32: 5-Axis Wireless Air Mouse

A custom-built, wearable/handheld spatial pointer that lets you control your computer or smartphone cursor wirelessly by tilting your hand in the air. 

This project features a custom software workaround using direct I2C communication via the `Wire` library to completely bypass a manufacturing defect (locked Gyro Z-axis) and broken register maps on a cloned MPU6500/6050 chip.

---

## ✨ Features
* **Bypassed Gyro Z Defect:** Re-engineered spatial tracking using the working Gyro X (Roll) for horizontal control and Gyro Y (Pitch) for vertical control.
* **No Heavy Libraries:** Built using pure `Wire.h` registry tracking, maximizing performance and keeping the ESP32 code memory footprint exceptionally low.
* **Low Memory BLE:** Powered by the `ESP32BLECombo` library for ultra-fast, RAM-friendly Bluetooth Low Energy HID performance.
* **Auto-Calibration:** Runs a baseline noise-absorb loop at startup to eliminate resting cursor drift.
* **Hardware Buttons:** Built-in internal pull-up logic tracking for physical Left and Right click inputs without needing external resistors.

---

## 🛠️ Hardware Requirements
* **Microcontroller:** ESP32 Development Board
* **Sensor:** MPU6050 / MPU6500 (Or equivalent cloned I2C IMU)
* **Buttons:** 2x Tactile Push Buttons (Left & Right Click)
* **Power:** 3.3V connection to prevent I2C bus logic level mismatching

---

## 🔌 Circuit Wiring Blueprint

| Component | Pin | ESP32 Target Pin | Function |
| :--- | :--- | :--- | :--- |
| **IMU Sensor** | VCC | **3V3** | 3.3V Main Power Line |
| | GND | **GND** | Shared System Ground |
| | SCL | **GPIO 22** | I2C Clock Line |
| | SDA | **GPIO 21** | I2C Data Line |
| | AD0 | **GND** | Hard-locks I2C Address to `0x68` |
| **Left Click** | Leg A / B | **GPIO 12 / GND** | Mouse Left Click |
| **Right Click**| Leg A / B | **GPIO 14 / GND** | Mouse Right Click |

---

## 💻 Software & Libraries Used
1. **Arduino IDE** 
2. **Wire.h** (Built-in I2C protocol library)
3. **ESP32BLECombo** (by kokodev) — An optimized BLE HID mouse/keyboard library utilizing `NimBLE`.

---

## 🚀 How It Works (The Roll-Pitch Mapping)
Standard air mice track horizontal movement via the **Yaw (Z-axis)**. Because this module had a locked Z-axis defect, the tracking configuration was re-mapped to operate seamlessly on 5 active axes:

1. **Vertical Movement (Y-Axis):** Handled natively by tilting the wrist up and down (**Pitch Axis**).
2. **Horizontal Movement (X-Axis):** Handled by tilting/banking the device sideways like an airplane (**Roll Axis**). This eliminates the need for Gyro Z entirely.

---

## 🔧 Installation & Setup
1. Clone this repository to your local machine.
2. Open the `.ino` file using your Arduino IDE.
3. Install the `ESP32BLECombo` library from the Library Manager.
4. Upload the code to your ESP32.
5. Open your Serial Monitor at **115200 Baud** and **keep the hardware completely still for 3 seconds** to complete the automatic drift calibration.
6. Open your PC or phone Bluetooth settings and pair with **"ESP32 Air Pointer"**.
