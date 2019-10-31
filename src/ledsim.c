#include "ledsim.h"

#include <assert.h>
#include <math.h>
#include <stdlib.h>

#include <SDL_events.h>
#include <SDL_video.h>
#include <SDL_opengl.h>

#include <stdio.h>

#ifdef __WIN32__
#include <Windows.h>
#else
#include <pthread.h>
#endif

#ifndef LEDSIM_LED_RADIUS
#define LEDSIM_LED_RADIUS 50
#endif

#ifndef LEDSIM_BTN_SIZE
#define LEDSIM_BTN_SIZE 100
#endif

#define LEDSIM_LED_BORDER (LEDSIM_LED_RADIUS / 5)
#define LEDSIM_BTN_MARGIN (LEDSIM_BTN_SIZE / 10)

#define LEDSIM_BTN_RELEASED_COLOR ((ledsim_color_t) {0xA0, 0xA0, 0xA0})
#define LEDSIM_BTN_HOVER_COLOR ((ledsim_color_t) {0xA0, 0xA0, 0xFF})
#define LEDSIM_BTN_PRESSED_COLOR ((ledsim_color_t) {0xA0, 0xFF, 0xA0})

static void ledsim_main_thread();
static void draw(void);
static void drawCircle(int x, int y, int radius, ledsim_color_t color);
static void drawRect(int x, int y, int sx, int sy, ledsim_color_t color);

#ifdef __WIN32__
DWORD WINAPI ledsim_win_thread(void* data) {
	ledsim_main_thread();
	return 0;
}
#else
void* ledsim_unix_thread(void* data) {
	ledsim_main_thread();
}
#endif

typedef enum {
	BUTTON_RELEASED,
	BUTTON_HOVER,
	BUTTON_PRESSED,
} ledsim_button_state_t;

typedef struct {
	int x;
	int y;
	int sx;
	int sy;
	ledsim_button_state_t state;
	ledsim_button_callback_t callback;
	void* userData;
} ledsim_button_handle_t;

static SDL_Window* window;
static SDL_GLContext context;

static ledsim_finish_callback_t finish_cb;

static uint32_t width;
static uint32_t height;
static uint32_t rows;
static uint32_t cols;

static int ledcount;
static int buttoncount;

static volatile ledsim_color_t* leds;
static volatile ledsim_button_handle_t* buttons;

static inline void ledsim_spawn_thread() {
#ifdef __WIN32__
	HANDLE thread = CreateThread(NULL, 0, ledsim_win_thread, NULL, 0, NULL);
#else
	pthread_t thread;
	pthread_create(&thread, NULL, ledsim_unix_thread, NULL);
#endif
}

int ledsim_start(uint32_t led_count, uint32_t r, uint32_t c, uint32_t button_count, ledsim_finish_callback_t callback) {
	assert(callback);
	assert(led_count <= r*c);

	finish_cb = callback;
	rows = r;
	cols = c;
	ledcount = led_count;
	buttoncount = button_count;

	width =  LEDSIM_LED_RADIUS * 2 * cols + ((cols + 1) * LEDSIM_LED_BORDER);
	height = LEDSIM_LED_RADIUS * 2 * rows + ((cols + 1) * LEDSIM_LED_BORDER) + 200;

	leds = (ledsim_color_t*) calloc(ledcount, sizeof(ledsim_color_t));
	buttons = (ledsim_button_handle_t*) calloc(buttoncount, sizeof(ledsim_button_handle_t));

	for (int i = 0; i < buttoncount; i++) {
		int posX = i * (LEDSIM_BTN_MARGIN + LEDSIM_BTN_SIZE) + LEDSIM_BTN_MARGIN;
		int posY = (1 + ((ledcount - 1) / cols)) * (LEDSIM_LED_BORDER + LEDSIM_LED_RADIUS * 2) + max(LEDSIM_LED_BORDER, LEDSIM_BTN_MARGIN);
		buttons[i].x = posX;
		buttons[i].y = posY;
		buttons[i].sx = LEDSIM_BTN_SIZE;
		buttons[i].sy = LEDSIM_BTN_SIZE;
	}

	ledsim_spawn_thread();

	return 0;
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

		if (e.type == SDL_MOUSEMOTION) {
			SDL_MouseMotionEvent mme = e.motion;
			for (int i = 0; i < buttoncount; i++) {
				if (mme.x > buttons[i].x && mme.x < buttons[i].x + buttons[i].sx &&
					mme.y > buttons[i].y && mme.y < buttons[i].y + buttons[i].sy) {
					if (buttons[i].state == BUTTON_PRESSED) continue;

					buttons[i].state = BUTTON_HOVER;
				}
				else {
					if (buttons[i].state != BUTTON_RELEASED) {
						buttons[i].callback(LEDSIM_BTN_RELEASED_FLAG, buttons[i].userData);
					}
					buttons[i].state = BUTTON_RELEASED;
				}
			}
		}

		if (e.type == SDL_MOUSEBUTTONDOWN && e.button.button == SDL_BUTTON_LEFT) {
			for (int i = 0; i < buttoncount; i++) {
				if (buttons[i].state == BUTTON_HOVER) {
					buttons[i].state = BUTTON_PRESSED;

					if (buttons[i].callback) {
						buttons[i].callback(LEDSIM_BTN_PRESSED_FLAG, buttons[i].userData);
					}
				}
			}
		}

		if (e.type == SDL_MOUSEBUTTONUP && e.button.button == SDL_BUTTON_LEFT) {
			for (int i = 0; i < buttoncount; i++) {
				if (buttons[i].state == BUTTON_PRESSED) {
					buttons[i].state = BUTTON_HOVER;

					if (buttons[i].callback) {
						buttons[i].callback(LEDSIM_BTN_RELEASED_FLAG, buttons[i].userData);
					}
				}
			}
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
		int posX = (i % cols) * (LEDSIM_LED_BORDER + LEDSIM_LED_RADIUS * 2) + LEDSIM_LED_BORDER + LEDSIM_LED_RADIUS;
		int posY = (i / cols) * (LEDSIM_LED_BORDER + LEDSIM_LED_RADIUS * 2) + LEDSIM_LED_BORDER + LEDSIM_LED_RADIUS;
		drawCircle(posX, posY, LEDSIM_LED_RADIUS, (ledsim_color_t) { 0xA0, 0xA0, 0xA0 });
		drawCircle(posX, posY, (int) (LEDSIM_LED_RADIUS * 0.9f), leds[i]);
	}

	for (int i = 0; i < buttoncount; i++) {
		ledsim_color_t color;
		switch (buttons[i].state)
		{
		case BUTTON_HOVER:
			color = LEDSIM_BTN_HOVER_COLOR;
			break;
		case BUTTON_PRESSED:
			color = LEDSIM_BTN_PRESSED_COLOR;
			break;
		default: // BUTTON_RELEASED
			color = LEDSIM_BTN_RELEASED_COLOR;
			break;
		}
		drawRect(buttons[i].x, buttons[i].y, buttons[i].sx, buttons[i].sy, color);
	}
}

/* Center of the circle is at (x,y) */
static void drawCircle(int x, int y, int radius, ledsim_color_t color) {
	glColor3f(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f);

	glBegin(GL_TRIANGLE_FAN);
	glVertex2f((GLfloat) x, (GLfloat) y);
	const static int res = 100;
	for (int i = 0; i <= res; i++) {
		glVertex2f(
			(GLfloat) (((float) x) + cos(i * 2 * M_PI / res) * ((float) radius)),
			(GLfloat) (((float) y) + sin(i * 2 * M_PI / res) * ((float) radius))
		);
	}
	glEnd();
}

static void drawRect(int x, int y, int sx, int sy, ledsim_color_t color) {
	glColor3f(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f);

	glBegin(GL_QUADS);
	glVertex2f((GLfloat) x, (GLfloat) y);
	glVertex2f((GLfloat) x + sx, (GLfloat) y);
	glVertex2f((GLfloat) x + sx, (GLfloat) (y + sy));
	glVertex2f((GLfloat) x, (GLfloat) (y + sy));
	glEnd();
}

void ledsim_setbuttoncallback(int index, ledsim_button_callback_t callback, void* userData) {
	assert(index < buttoncount);

	buttons[index].callback = callback;
	buttons[index].userData = userData;
}
