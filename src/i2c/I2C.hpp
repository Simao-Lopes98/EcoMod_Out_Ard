#pragma once

#include <Arduino.h>
#include <Wire.h>
#include "../env.hpp"
#include "../queues/queues.hpp"

namespace I2C
{
    void setup_i2c();
    String read_i2c_sensor(int add);
    
    void taskI2C(void *pvParameters);
}