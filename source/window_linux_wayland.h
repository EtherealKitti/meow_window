#pragma once

#include "window_linux_wayland.h"
#include "../include/window.h"
#include <pthread.h>
#include <meow_utilities/include/meow_math.h>
#include "vulkan.h"
#include "../platformSpecificSourceFiles/Linux/xdg-shell-client-protocol.h"

struct pointer_wayland {
    meow_vector2 position;
    meow_vector2 velocity;
};

struct keyboard_wayland {
    uint32_t keymapFormat;
    int keymapFileDescriptor;
    uint32_t keymapSize;
    struct xkb_keymap *keymap;
    struct xkb_state *keymapState;
    struct xkb_context *keymapContext;
};

struct window_wayland {
    struct meow_windowProperties windowProperties;
    struct wl_registry *waylandRegistry;
    struct wl_registry_listener waylandRegistryListener;
    struct wl_compositor *waylandCompositor;
    struct xdg_wm_base *xdgWindowManagerBase;
    struct wl_seat *waylandSeat;
    struct xdg_wm_base_listener xdgWindowManagerBaseListener;
    struct pointer_wayland pointer;
    struct wl_pointer_listener waylandPointerListener;
    struct keyboard_wayland keyboard;
    struct wl_keyboard_listener waylandKeyboardListener;
    struct wl_surface *waylandSurface;
    struct xdg_surface *xdgSurface;
    struct xdg_toplevel *xdgToplevelWindow;
    struct xdg_toplevel_listener xdgToplevelListener;
    struct xdg_surface_listener xdgSurfaceListener;
    struct vulkanContext vulkanContext;
};

struct window_wayland *createWindow_wayland(char *name,meow_vector2 size,meow_windowType type);

_Bool windowIsRunning_wayland(struct window_wayland *window);

void dispatchEvents_wayland();

void renderWindowFrame_wayland(struct window_wayland *window);

void destroyWindow_wayland(struct window_wayland *window);
