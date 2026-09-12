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

## 🚀 How It Works (The Roll-Pitch Mapping)
Standard air mice track horizontal movement via the **Yaw (Z-axis)**. Because this module had a locked Z-axis defect, the tracking configuration was re-mapped to operate seamlessly on 5 active axes:

1. **Vertical Movement (Y-Axis):** Handled natively by tilting the wrist up and down (**Pitch Axis**).
2. **Horizontal Movement (X-Axis):** Handled by tilting/banking the device sideways like an airplane (**Roll Axis**). This eliminates the need for Gyro Z entirely.

---

## 🛠️ Hardware Requirements

|Sr no. | Componets | Quantity | Description |
|---|---|---|---|
| 1 | ESP32 | 1 | Brain of cursor working | 
| 2 | Pushbutton | 4 | For clicks | 
| 3 | Female pin headers | 2 | To place microcontroller and mpu6050 |
| 4 | Lm2596 buck converter | 1 | For power supply 5v | 
| 5 | white (5mm) led | 1 | To indicate that esp32 and mpu6050 is working | 
| 6 | red (5mm) led | 1 | if battery voltage low it will light up  | 
| 7 | Switch | 1 | to turn ON or OFF | 
| 8 | 2x lithium battery holder | 1 | to place 2 lithium ion batteries |

---

## 🧰 Required Equipment & Tools

1. 30 AWG Wrapping Wire: Essential for neat, ultra-fine point-to-point row and column routing underneath your perfboard.
2. Soldering Iron & Stand: A fine-tip soldering iron set to a steady temperature is necessary to handle.
3. Rosin Core Solder Wire: High-quality thin solder wire to ensure clean, flux-filled solder joints that resist cracking during movement.
4. Protoboard / Perfboard: The structural base canvas used to arrange and hand-solder the components into a single standalone device.

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

## 📸 Hardware photos

| Front | Back |
|:---:|:---:|
| ![Front image](docs/images/front.jpg) | ![Back image](docs/images/back.jpg) | 

---

## 🎬 Working Demo 

![GIF](docs/images/demo.gif)

---

## 🔌 Wiring / Connections

Schematics
|:---:|
|![Wiring diagram](docs/images/schematic.jpeg)|


---

## 🛠️ Guide to Build It

Follow these steps to assemble the hardware and flash the software for your custom Air Mouse.

### 1. Hardware Assembly & Connections
Before wiring the components, check out the wiring schematic image/diagram file included directly inside this repository. 

Key physical rules to keep in mind during assembly:
* **Power Supply:** Connect the IMU's `VCC` pin directly to the ESP32's **3.3V (3V3) pin**. Avoid the 5V line to prevent logic level mismatches that can freeze the I2C communication bus.
* **No External Resistors:** You do not need external pull-up resistors for the tactile push buttons. The internal `INPUT_PULLUP` resistors on the ESP32 pins are enabled automatically by the firmware code.
* **Shared Grounds:** Ensure all component ground points (the IMU and both click buttons) are tied together back to a common **GND** pin on the ESP32 dev board.
* **Address Pin:** Tie the **AD0** pin on your IMU sensor directly to **GND** to permanently lock its I2C address profile to `0x68`.

---

### 2. Software Installation & Upload
You can set up the codebase on your system using one of the following two options:

#### Option A: Clone the Repository (Recommended)
If you have Git installed, open your terminal or command prompt and run the following command to download the entire project workspace directly:
```bash
git clone https://github.com
```

#### Option B: Manual Copy-Paste
1. Open the **Arduino IDE** on your computer.
2. Create a new sketch (`File` > `New Sketch`).
3. Delete any default placeholder code.
4. Copy the entire source code file provided in this repository (e.g., `Wireless_cursor.ino`) and paste it directly into your new sketch window.

---

### 3. Final Firmware Configuration
Before hitting the upload button, complete these quick environment setup configurations:

1. **Install Dependencies:** Go to `Tools` > `Manage Libraries` in the Arduino IDE. Search for **ESP32BLECombo** (by kokodev) and install it.
2. **Board Selection:** Select your specific ESP32 module under `Tools` > `Board` > `esp32`.
3. **Flash Code:** Connect your ESP32 to your PC using a reliable data-capable USB cable and click the **Upload** arrow icon.
4. **Initial Calibration:** Once uploaded, open the Serial Monitor (`115200` Baud). **Keep your hardware completely flat and motionless for the first 3 seconds** while the system automatically calculates the drift offsets to zero out resting cursor noise.

---

## Repository Structure

```
├── docs/images
├── Wireless_cursor/
│   Wireless_cursor.ino
├── LICENSE
└── README.md

```

## 📄 License

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.

## 👤 Author

**Soham Kale**
GitHub: [@SohamKale83](https://github.com/SohamKale83)
