#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

#include "ledsim.h"

volatile bool finishing = false;

void onFinish(void) {
	finishing = true;
}

int main()
{
	int led_per_row = 15;
	ledsim_start(led_per_row*led_per_row, led_per_row, led_per_row, onFinish);

	ledsim_setled(23, (ledsim_color_t) { 0x00, 0xFF, 0x00 });
	ledsim_setled(25, (ledsim_color_t) { 0x00, 0x00, 0xFF });
	ledsim_setled(26, (ledsim_color_t) { 0xFF, 0x00, 0xFF });

	while (!finishing) {
	}

	return 0;
}