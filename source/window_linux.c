#ifdef __linux__

#include "../include/window.h"
#include "window_linux_wayland.h"

static const _Bool usingWayland() {
    return 1;
}

meow_window *meow_createWindow(char *title,meow_vector2 size,meow_windowType type) {
    if (usingWayland()) {
        return (meow_window *)createWindow_wayland(title,size,type);
    }
}

_Bool meow_windowIsRunning(meow_window *window) {
    if (usingWayland()) {
        return windowIsRunning_wayland((struct window_wayland *)window);
    }
}

void meow_dispatchEvents() {
    if (usingWayland()) {
        dispatchEvents_wayland();
    }
}

void meow_renderWindowFrame(meow_window *window) {
    if (usingWayland()) {
        renderWindowFrame_wayland((struct window_wayland *)window);
    }
}

void meow_destroyWindow(meow_window *window) {
    if (usingWayland()) {
        destroyWindow_wayland((struct window_wayland *)window);
    }
}

#endif
