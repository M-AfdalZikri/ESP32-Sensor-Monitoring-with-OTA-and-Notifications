# ESP32-Sensor-Monitoring-with-OTA-and-Notifications

## Description
This device uses the **ESP32** connected to **gas sensors** and **fire sensors** to detect gas leaks and fires. It also includes a **PIR sensor** to detect movement, which can be dynamically added via **Over-The-Air (OTA)** updates once the device is connected to the ESP32.

### Device Components
- **Gas Sensor**: Detects the presence of gas leaks that could pose a danger to the environment.
- **Fire Sensor**: Detects the presence of fire or combustion to provide early warning.
- **PIR Sensor (Passive Infrared)**: Detects movement around the device. This sensor can be added after an OTA update is performed on the ESP32.

### OTA (Over-The-Air) Feature
The **OTA (Over-The-Air)** feature allows the device to update its program or firmware remotely without needing a physical connection. In this case, although only the **gas sensor** and **fire sensor** are initially installed, the device can add the **PIR sensor** after an OTA update.

#### OTA Update Process:
1. **Initial Setup**: When the device is first set up, only the gas and fire sensors are active. The uploaded program only supports these two sensors.
2. **OTA Update**: After the device is connected to Wi-Fi and the ESP32, the program update is performed via OTA without the need for a direct physical connection.
3. **Adding PIR Sensor**: Once the update is successfully uploaded, the program will automatically activate the PIR sensor on the device. **However, ensure that the device is connected to the ESP32 before performing the OTA update** to ensure the PIR sensor is added correctly.

### Benefits of OTA Feature
- **Flexibility**: Device updates can be done remotely without requiring direct configuration or physical intervention.
- **Feature Upgrades**: With OTA, users can add or update sensors and features based on their needs without the hassle of changing hardware.
- **Remote Maintenance**: Users can fix bugs or perform software updates without visiting the device's physical location.

## Applications
- Ideal for home or building security systems that need to detect gas leaks, fire, and movement, and require easy maintenance and device updates.

## Technologies Used
- **ESP32** for processing and Wi-Fi connectivity.
- **Gas Sensor** and **Fire Sensor** for hazard detection.
- **PIR Sensor** for motion detection.
- **OTA** for remote program updates.

## Installation and Usage
1. Install the ESP32 device at the desired location and connect it to the gas and fire sensors.
2. Connect the device to an available Wi-Fi network.
3. Ensure the device is connected to the ESP32.
4. Perform the OTA update to activate the PIR sensor.
5. Monitor the device via the installed system.

## Future Enhancements
- Adding other sensors to enhance hazard detection capabilities.
- Integration with notification systems for quicker alerts.

---
With the **OTA** feature, this device offers convenience and efficiency in maintenance and functionality upgrades remotely, allowing new sensors like the PIR to be added without replacing hardware, as long as the device is connected to the ESP32.

