# Self-Balancing Two-Wheeled Robot (ESP32)

A real-time embedded systems project featuring a two-wheeled balancing robot built on the **ESP32** platform using **PlatformIO** and **C++**. The system combines advanced control theory, multi-core processing, and an interactive web-based configuration dashboard to deliver stable hardware control and intuitive tuning.

## 🚀 Key Features

* **Real-Time Balancing:** Implements a closed-loop **PID Controller** to dynamically adjust motor outputs and maintain vertical stability.
* **Full-Stack Embedded Webserver:** Hosts a local HTTP server directly from the ESP32 featuring **dynamic, customizable tables** for live telemetry and on-the-fly PID tuning.
* **Sensor Fusion & Filtering:** Integrates an **MPU6050 IMU** (Inertial Measurement Unit) to sample accelerometer and gyroscope data, combining them to compute precise, low-noise tilt angles.
* **Wireless Remote Control:** Integrated **PS4 Controller** connectivity via Bluetooth for real-time steering and physical overrides.
* **Dual-Core Execution:** Optimized for the ESP32 architecture, separating time-critical PID/Sensor loops from peripheral handling (Webserver and Bluetooth communication).

## 📊 Feature Deep-Dive: Customizable Webserver & Telemetry

To eliminate the need for constant flashing during calibration, the firmware spins up an asynchronous webserver on the ESP32’s Wi-Fi Access Point/Station mode. 

### Core Web Capabilities:
* **Dynamic Parameter Tables:** A responsive front-end dashboard that displays active system variables (e.g., $K_p$, $K_i$, $K_d$, Target Angle, Battery Voltage) inside customizable HTML/CSS tables.
* **On-the-Fly Tuning:** Users can input new coefficients directly into the web interface. The server parses the incoming POST requests/WebSockets and updates the PID variables in flash memory (EEPROM/Preferences) dynamically without resetting the robot.
* **Asynchronous Architecture:** Built using an asynchronous network stack to ensure handling incoming HTTP requests never blocks or interrupts the ultra-high frequency balance loop.

## 🛠️ Tech Stack & Hardware

* **Framework:** PlatformIO (Arduino-ESP32 Core)
* **Language:** Modern C++, HTML5, CSS3, JavaScript (ES6)
* **Microcontroller:** ESP32 (Dual-core, Wi-Fi & Bluetooth)
* **Sensors:** MPU6050 (3-Axis Gyroscope & 3-Axis Accelerometer)
* **Actuators & Controllers:** Dual DC Motors with high-resolution encoders / Motor Drivers
* **Key Libraries:** * `ESPAsyncWebServer` or `WebServer` (For the telemetry dashboard)
  * `PS4Controller` (Bluetooth HID pairing)
  * `Wire` (I2C communication for IMU)

## 📂 Project Structure

```text
CodePlatformIO/
├── include/          # Header files (.h) for Webserver, PID, and Sensor configs
├── src/              # Main application logic (.cpp)
│   ├── main.cpp      # Core execution loop (Task creation, Core assignment)
│   ├── webserver.cpp # HTTP Handling, URI routing, and HTML table injection
│   └── ...           # Peripherals, Filtering, and Controller implementation
└── platformio.ini    # Environment configurations, baud rates, and library dependencies
