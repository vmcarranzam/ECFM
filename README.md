# ECFM Physics Kart
Repository for the Physics Kart project for the School of Physics and Mathematics Sciences (EFCM) from Universidad de San Carlos de Guatemala.

Physics kart is basically a set of sensors mounted on a PCB and case with wheels. A microcontroller is used to get the data output from each sensor, process it and send it through a WiFi connection to a server or broker via the MQTT protocol for IoT in seemingly real time. This data can be read and further processed to be displayed graphically and be a visual aid for physics lab experiments.

## Controller module and other used libraries
- DOIT ESP32 DEVKIT V1 (Installation and requirements guide to program using Arduino IDE: https://github.com/espressif/arduino-esp32)
- For MQTT protocol implementation: https://github.com/knolleary/pubsubclient
- For WiFi connection handling:https://github.com/espressif/arduino-esp32/tree/master/libraries/WiFi

## List of sensor modules and their respective libraries
- MPU6050 module to measure acceleration and orientation in XYZ (Library used: https://github.com/tockn/MPU6050_tockn/blob/master/README.md)
- 20 Kg Load Cell with HX711 module to measure frontal collisions (Library used: https://github.com/bogde/HX711)
- Optical wheel based rotary encoder to measure angular speed and position

## Last updates

Uploaded PCB design files for the first prototype. Created using KiCAD.
