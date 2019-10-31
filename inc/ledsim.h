#ifndef LEDSIM_H
#define LEDSIM_H

#include <stdint.h>

#define LEDSIM_BTN_PRESSED_FLAG (1 << 0)
#define LEDSIM_BTN_RELEASED_FLAG (1 << 1)

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} ledsim_color_t;

typedef void(*ledsim_finish_callback_t)(void);

typedef void(*ledsim_button_callback_t)(int flags, void* userData);

int ledsim_start(uint32_t led_count, uint32_t rows, uint32_t cols, uint32_t button_count, ledsim_finish_callback_t callback);

void ledsim_setled(int index, ledsim_color_t color);

void ledsim_setbuttoncallback(int index, ledsim_button_callback_t callback, void* userData);

#endif
