// pcal6416a.c
#include "pcal6416a.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"

// Register addresses
#define REG_INPUT_PORT_0        0x00
#define REG_OUTPUT_PORT_0       0x02
#define REG_CONFIG_PORT_0       0x06
#define REG_OUTPUT_MODE         0x4F
#define REG_PULL_ENABLE_PORT_0  0x46
#define REG_PULL_SELECT_PORT_0  0x48

// Helper to write to a register
static void write_reg(uint8_t reg, uint8_t val) {
    uint8_t buf[2] = {reg, val};
    i2c_write_blocking(PCAL6416A_I2C, PCAL6416A_ADDR, buf, 2, false);
}

// Helper to read from a register
static uint8_t read_reg(uint8_t reg) {
    i2c_write_blocking(PCAL6416A_I2C, PCAL6416A_ADDR, &reg, 1, true);
    uint8_t val = 0;
    i2c_read_blocking(PCAL6416A_I2C, PCAL6416A_ADDR, &val, 1, false);
    return val;
}

void pcal6416a_init() {
    i2c_init(PCAL6416A_I2C, 100 * 1000);
    gpio_set_function(PCAL6416A_SDA_PIN, GPIO_FUNC_I2C);
    gpio_set_function(PCAL6416A_SCL_PIN, GPIO_FUNC_I2C);
    gpio_pull_up(PCAL6416A_SDA_PIN);
    gpio_pull_up(PCAL6416A_SCL_PIN);

    // Configure outputs and inputs
    write_reg(REG_OUTPUT_MODE, 0x00);              // Push-pull mode
    write_reg(REG_PULL_ENABLE_PORT_0, 0b01110111); // Pull-up on P0_0–P0_2, P0_4–P0_6
    write_reg(REG_PULL_SELECT_PORT_0, 0b01110111); // Select pull-up
    write_reg(REG_CONFIG_PORT_0, 0b01110000);      // P0_0–P0_2 = outputs, P0_4–P0_6 = inputs
}

void pcal6416a_led_off()    { write_reg(REG_OUTPUT_PORT_0, 0b00000000); }
void pcal6416a_led_green()  { write_reg(REG_OUTPUT_PORT_0, 0b00000001); }
void pcal6416a_led_blue()   { write_reg(REG_OUTPUT_PORT_0, 0b00000010); }
void pcal6416a_led_red()    { write_reg(REG_OUTPUT_PORT_0, 0b00000100); }
void pcal6416a_led_purple() { write_reg(REG_OUTPUT_PORT_0, 0b00000110); }

uint8_t pcal6416a_read_buttons() {
    return read_reg(REG_INPUT_PORT_0) & 0b01110000; // Return P0_4–P0_6 only
}
