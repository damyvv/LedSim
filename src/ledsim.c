#include "ledsim.h"

#include <assert.h>
#include <math.h>
#include <string.h>

#include <SDL.h>
#include <SDL_opengl.h>

#ifdef _MSC_BUILD
#include <Windows.h>
#else
#include <pthread.h>
#endif

#define LEDSIM_BORDER (LEDSIM_LED_RADIUS / 5)

static void ledsim_main_thread();
static void draw(void);
static void drawCircle(int x, int y, int radius, ledsim_color_t color);

#ifdef _MSC_BUILD
DWORD WINAPI ledsim_win_thread(void* data) {
	ledsim_main_thread();
}
#else
void* ledsim_unix_thread(void* data) {
	ledsim_main_thread();
}
#endif

static SDL_Window* window;
static SDL_GLContext context;

static ledsim_finish_callback finish_cb;

static uint32_t width;
static uint32_t height;
static uint32_t rows;
static uint32_t cols;

static int ledcount;

volatile ledsim_color_t* leds;

static inline void ledsim_spawn_thread() {
#ifdef _MSC_BUILD
	HANDLE thread = CreateThread(NULL, 0, ledsim_win_thread, NULL, 0, NULL);
#else
	pthread_t thread;
	pthread_create(&thread, NULL, ledsim_unix_thread, NULL);
#endif
}

int ledsim_start(uint32_t led_count, uint32_t r, uint32_t c, ledsim_finish_callback callback) {
	assert(callback);
	assert(led_count <= r*c);

	finish_cb = callback;
	rows = r;
	cols = c;
	ledcount = led_count;

	width =  LEDSIM_LED_RADIUS * 2 * cols + ((cols + 1) * LEDSIM_BORDER);
	height = LEDSIM_LED_RADIUS * 2 * rows + ((cols + 1) * LEDSIM_BORDER);

	size_t bytes = sizeof(ledsim_color_t) * ledcount;
	leds = malloc(bytes);
	memset((void*) leds, 0, bytes);

	ledsim_spawn_thread();
}

void ledsim_setled(int index, ledsim_color_t color) {
	assert(index < ledcount);
	leds[index] = color;
}

static void ledsim_main_thread() {
	window = SDL_CreateWindow("LedSim", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
	context = SDL_GL_CreateContext(window);

	SDL_GL_SetSwapInterval(1);

	if (context == NULL || window == NULL) {
		assert(0);
		return;
	}

	glOrtho(0, width, height, 0, -1, 1);

	while (1) {
		SDL_Event e;
		SDL_PollEvent(&e);

		if (e.type == SDL_QUIT) {
			break;
		}

		// Clear screen
		glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		draw();

		SDL_GL_SwapWindow(window);
	}

	SDL_GL_DeleteContext(context);

	finish_cb();
}

static void draw(void) {
	for (int i = 0; i < ledcount; i++) {
		int posX = (i % cols) * (LEDSIM_BORDER + LEDSIM_LED_RADIUS * 2) + LEDSIM_BORDER + LEDSIM_LED_RADIUS;
		int posY = (i / cols) * (LEDSIM_BORDER + LEDSIM_LED_RADIUS * 2) + LEDSIM_BORDER + LEDSIM_LED_RADIUS;
		drawCircle(posX, posY, LEDSIM_LED_RADIUS, (ledsim_color_t) { 0xA0, 0xA0, 0xA0 });
		drawCircle(posX, posY, LEDSIM_LED_RADIUS *0.9f, leds[i]);
	}
}

/* Center of the circle is at (x,y) */
static void drawCircle(int x, int y, int radius, ledsim_color_t color) {
	glColor3f(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f);

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f(x, y);
	const static int res = 100;
	for (int i = 0; i <= res; i++) {
		glVertex2f(
			((float) x) + cos(i * 2 * M_PI / res) * ((float) radius),
			((float) y) + sin(i * 2 * M_PI / res) * ((float) radius)
		);
	}
	glEnd();
}
