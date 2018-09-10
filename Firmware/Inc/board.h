#ifndef __BOARD_H
#define __BOARD_H


#include "HTS221.h"
#include "lsm303d.h"
#include "micronet.h"

void set_fet_pwm(unsigned char switch_id, unsigned char duty);
void beep(unsigned int width);
void board_process(void);
void board_init(void);
void beep(unsigned int width);
void board_led(char led_state);
uint16_t get_board_voltage(void);
void fet_set_state(char fet, char state);
void raspberry_power(char power_state);

#endif
