// stereo USB microphone firmware (Pico + TinyUSB) with proper ready-state guards + raw buffer reuse protection

#define SAMPLE_RATE 12000
#define SAMPLE_BUFFER_SIZE 12
#define INTERLEAVED_SIZE (SAMPLE_BUFFER_SIZE * 2)

#include "pico/pdm_microphone.h"
#include "usb_microphone.h"
#include "pico/stdlib.h"
#include "hardware/watchdog.h"
#include "pico/multicore.h"
#include <stdbool.h>
#include <stdint.h>
#include "pcal6416a.h"

#include "pico/time.h"
absolute_time_t last_tx_time;
#define USB_FRAME_MS 1

// To Do ...
// Implement I2C control of the RGB LED onboard to provide a status indication.
// Can one of the buttons be reassigned to BOOTSEL?

// Microphone configuration
const struct pdm_microphone_config config_left = {
    .gpio_data = 2,
    .gpio_clk  = 3,
    .pio       = pio0,
    .pio_sm    = 0,
    .sample_rate = SAMPLE_RATE,
    .sample_buffer_size = SAMPLE_BUFFER_SIZE,
};

const struct pdm_microphone_config config_right = {
    .gpio_data = 4,
    .gpio_clk  = 5,
    .pio       = pio0,
    .pio_sm    = 1,
    .sample_rate = SAMPLE_RATE,
    .sample_buffer_size = SAMPLE_BUFFER_SIZE,
};

// Sample buffers
int16_t sample_buffer[2][SAMPLE_BUFFER_SIZE];
int16_t stereo_interleaved_buffer[INTERLEAVED_SIZE];

// State flags
volatile bool left_ready = false;
volatile bool right_ready = false;
volatile bool stereo_ready = false;

// Last buffer index read (per mic)
int last_read_index[2] = {-1, -1};

void software_reset() {
    watchdog_enable(1, 1);
    while (1) {}
}

void on_pdm_samples_ready(int mic_num);
void on_pdm_samples_ready_left(void) { on_pdm_samples_ready(0); }
void on_pdm_samples_ready_right(void) { on_pdm_samples_ready(1); }

extern int pdm_microphone_read(int16_t* buffer, size_t samples, int mic_num);

void on_pdm_samples_ready(int mic_num) {
    int samples = pdm_microphone_read(sample_buffer[mic_num], SAMPLE_BUFFER_SIZE, mic_num);

    if (samples > 0) {
        if (mic_num == 0) left_ready = true;
        if (mic_num == 1) right_ready = true;
    }

    if (left_ready && right_ready) {
        stereo_ready = true;
    }
}

void on_usb_microphone_tx_ready(void) {
    static uint32_t tx_count = 0;

    if (!stereo_ready) {
        // Fill with silence if data isn't ready in time
        for (int i = 0; i < INTERLEAVED_SIZE; i++) {
            stereo_interleaved_buffer[i] = 0;
        }
    } else {
        for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++) {
            stereo_interleaved_buffer[2 * i]     = sample_buffer[0][i];  // Left
            stereo_interleaved_buffer[2 * i + 1] = sample_buffer[1][i];  // Right
        }
    }

    usb_microphone_write((uint8_t *)stereo_interleaved_buffer, 48);

    // Clear flags regardless
    left_ready = false;
    right_ready = false;
    stereo_ready = false;

    tx_count++;
}


void pdm_microphone_data_enable(PIO pio) {
    pio->ctrl |= 0b0011; // Enable SM0 + SM1
}

void core1_entry(void) {
    while (1) {
        usb_microphone_task();
    }
}

int main(void) {
    stdio_init_all();

    //sleep_ms(100);
    pcal6416a_init();
    //sleep_ms(10);
    pcal6416a_led_purple();
    

    last_tx_time = get_absolute_time();


    if (pdm_microphone_init(&config_left, 0) < 0) software_reset();
    if (pdm_microphone_init(&config_right, 1) < 0) software_reset();

    pdm_microphone_set_samples_ready_handler(on_pdm_samples_ready_left, 0);
    pdm_microphone_set_samples_ready_handler(on_pdm_samples_ready_right, 1);

    if (pdm_microphone_start(0) < 0) software_reset();
    if (pdm_microphone_start(1) < 0) software_reset();

    pdm_microphone_data_enable(pio0);

    usb_microphone_init();
    usb_microphone_set_tx_ready_handler(on_usb_microphone_tx_ready);

    multicore_launch_core1(core1_entry);

    while (1) {
        tight_loop_contents();
    }

    return 0;
}
