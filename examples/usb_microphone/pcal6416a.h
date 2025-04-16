// pcal6416a.h
#ifndef PCAL6416A_H
#define PCAL6416A_H

#include "hardware/i2c.h"
#include <stdint.h>

#define PCAL6416A_ADDR 0x20  // Default I2C address
#define PCAL6416A_I2C i2c1
#define PCAL6416A_SDA_PIN 26
#define PCAL6416A_SCL_PIN 27

void pcal6416a_init();
void pcal6416a_led_off();
void pcal6416a_led_green();
void pcal6416a_led_blue();
void pcal6416a_led_red();
void pcal6416a_led_purple();
uint8_t pcal6416a_read_buttons();

#endif // PCAL6416A_H
