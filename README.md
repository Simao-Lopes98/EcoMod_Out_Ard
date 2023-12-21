# EcoModZHC Output Source Code
Source code for the output MCU for the masters final project.

## Introduction
Source code for ESP32, written mainly in C++, using a mix of Arduino and IDF frameworks and using the FreeRTOS scheduler.

In this project the ESP32 on the output of the system is responsible for acquiring data from sensors and relaying it to an MQTT broker.

## Project tree
This project is divided in services, each one having a .cpp and .hpp file.

TODO

## Technologies
In order receive data form the sensors a lot of communication protocols were used:

* I2C
* MQTT
* ModBus
* UART
* HTTP


