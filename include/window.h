#pragma once

#include <meow_utilities/include/meow_math.h>

typedef enum {
    MEOW_WINDOW_TYPE_TOPLEVEL,
    MEOW_WINDOW_TYPE_POPUP,
    MEOW_WINDOW_TYPE_DIALOG,
    MEOW_WINDOW_TYPE_UTILITY
} meow_windowType;

typedef struct meow_pointer meow_pointer;

typedef struct meow_keyboard meow_keyboard;

struct meow_windowProperties {
    char *name;
    uint8_t monitor;
    meow_vector2 size;
    _Bool maximized;
    _Bool fullscreen;
    meow_windowType type;
    _Bool running;
};

typedef struct meow_window meow_window;

meow_window *meow_createWindow(char *title,meow_vector2 size,meow_windowType type);

_Bool meow_windowIsRunning(meow_window *window);

void meow_dispatchEvents();

void meow_renderWindowFrame(meow_window *window);

void meow_destroyWindow(meow_window *window);
