#ifndef LEDSIM_H
#define LEDSIM_H

#ifndef LEDSIM_LED_RADIUS
#define LEDSIM_LED_RADIUS 20
#endif

#include <stdint.h>

typedef struct {
	uint8_t r;
	uint8_t g;
	uint8_t b;
} ledsim_color_t;

typedef void(*ledsim_finish_callback)(void);

int ledsim_start(uint32_t led_count, uint32_t rows, uint32_t cols, ledsim_finish_callback callback);

void ledsim_setled(int index, ledsim_color_t color);

#endif
