/*
Copyright 2012 Jun Wako <wakojun@gmail.com>
Copyright 2013 Oleg Kostyuk <cub.uanic@gmail.com>
Copyright 2015 ZSA Technology Labs Inc (@zsa)
Copyright 2020 Christopher Courtney <drashna@live.com> (@drashna)

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "ergodox_fn.h"

keyboard_config_t keyboard_config;

bool         i2c_initialized = 0;
i2c_status_t mcp23018_status = 0x20;

void matrix_init_kb(void) {
    // keyboard LEDs (see "PWM on ports OC1(A|B|C)" in "teensy-2-0.md")
    TCCR1A = 0b10101001;  // set and configure fast PWM
    TCCR1B = 0b00001001;  // set and configure fast PWM

    // (tied to Vcc for hardware convenience)
    DDRB &= ~(1 << 4);   // set B(4) as input
    PORTB &= ~(1 << 4);  // set B(4) internal pull-up disabled

    // unused pins - C7, D4, D5, D7, E6
    // set as input with internal pull-up enabled
    DDRC &= ~(1 << 7);
    DDRD &= ~(1 << 5 | 1 << 4);
    DDRE &= ~(1 << 6);
    PORTC |= (1 << 7);
    PORTD |= (1 << 5 | 1 << 4);
    PORTE |= (1 << 6);

    keyboard_config.raw = eeconfig_read_kb();

    matrix_init_user();
}

uint8_t init_mcp23018(void) {
    mcp23018_status = 0x20;

    // I2C subsystem

    // uint8_t sreg_prev;
    // sreg_prev=SREG;
    // cli();

    if (i2c_initialized == 0) {
        i2c_init();  // on pins D(1,0)
        i2c_initialized = true;
        _delay_ms(1000);
    }
    // i2c_init(); // on pins D(1,0)
    // _delay_ms(1000);

    // set pin direction
    // - unused  : input  : 1
    // - input   : input  : 1
    // - driving : output : 0
    mcp23018_status = i2c_start(I2C_ADDR_WRITE, ERGODOX_FN_I2C_TIMEOUT);
        if (mcp23018_status) goto out;
    mcp23018_status = i2c_write(IODIRA, ERGODOX_FN_I2C_TIMEOUT);
        if (mcp23018_status) goto out;
    mcp23018_status = i2c_write(0b00000000, ERGODOX_FN_I2C_TIMEOUT);
        if (mcp23018_status) goto out;
    mcp23018_status = i2c_write(0b00111111, ERGODOX_FN_I2C_TIMEOUT);
        if (mcp23018_status) goto out;
    i2c_stop();

    // set pull-up
    // - unused  : on  : 1
    // - input   : on  : 1
    // - driving : off : 0
    mcp23018_status = i2c_start(I2C_ADDR_WRITE, ERGODOX_FN_I2C_TIMEOUT);
    if (mcp23018_status) goto out;
    mcp23018_status = i2c_write(GPPUA, ERGODOX_FN_I2C_TIMEOUT);
    if (mcp23018_status) goto out;
    mcp23018_status = i2c_write(0b00000000, ERGODOX_FN_I2C_TIMEOUT);
    if (mcp23018_status) goto out;
    mcp23018_status = i2c_write(0b00111111, ERGODOX_FN_I2C_TIMEOUT);
    if (mcp23018_status) goto out;

out:
    i2c_stop();

    // SREG=sreg_prev;

    return mcp23018_status;
}


#ifdef SWAP_HANDS_ENABLE
__attribute__((weak))
// swap-hands action needs a matrix to define the swap
const keypos_t PROGMEM hand_swap_config[MATRIX_ROWS][MATRIX_COLS] = {
    /* Left hand, matrix positions */
    {{0, 13}, {1, 13}, {2, 13}, {3, 13}, {4, 13}, {5, 13}},
    {{0, 12}, {1, 12}, {2, 12}, {3, 12}, {4, 12}, {5, 12}},
    {{0, 11}, {1, 11}, {2, 11}, {3, 11}, {4, 11}, {5, 11}},
    {{0, 10}, {1, 10}, {2, 10}, {3, 10}, {4, 10}, {5, 10}},
    {{0, 9}, {1, 9}, {2, 9}, {3, 9}, {4, 9}, {5, 9}},
    {{0, 8}, {1, 8}, {2, 8}, {3, 8}, {4, 8}, {5, 8}},
    {{0, 7}, {1, 7}, {2, 7}, {3, 7}, {4, 7}, {5, 7}},
    /* Right hand, matrix positions */
    {{0, 6}, {1, 6}, {2, 6}, {3, 6}, {4, 6}, {5, 6}},
    {{0, 5}, {1, 5}, {2, 5}, {3, 5}, {4, 5}, {5, 5}},
    {{0, 4}, {1, 4}, {2, 4}, {3, 4}, {4, 4}, {5, 4}},
    {{0, 3}, {1, 3}, {2, 3}, {3, 3}, {4, 3}, {5, 3}},
    {{0, 2}, {1, 2}, {2, 2}, {3, 2}, {4, 2}, {5, 2}},
    {{0, 1}, {1, 1}, {2, 1}, {3, 1}, {4, 1}, {5, 1}},
    {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {5, 0}},
};
#endif

void eeconfig_init_kb(void) {  // EEPROM is getting reset!
    keyboard_config.raw               = 0;
    keyboard_config.led_level         = 4;
    keyboard_config.rgb_matrix_enable = true;
    eeconfig_update_kb(keyboard_config.raw);
    eeconfig_init_user();
}

#ifdef DYNAMIC_MACRO_ENABLE
static bool is_dynamic_recording = false;
static uint16_t dynamic_loop_timer;

void dynamic_macro_record_start_user(void) {
    is_dynamic_recording = true;
    dynamic_loop_timer = timer_read();
}

void dynamic_macro_record_end_user(int8_t direction) {
    is_dynamic_recording = false;
    layer_state_set_user(layer_state);
}
#endif

void matrix_scan_kb(void) {
    matrix_scan_user();
}
