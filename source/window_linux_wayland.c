#ifdef __linux__

#include "window_linux_wayland.h"
#include "../include/window.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <wayland-client.h>
#include <vulkan/vulkan_wayland.h>
#include <sys/mman.h>
#include <xkbcommon/xkbcommon.h>
#include "../platformSpecificSourceFiles/Linux/xdg-shell-client-protocol.h"
#include <meow_utilities/include/log.h>
#include <meow_utilities/include/meow_math.h>
#include "vulkan.h"

static struct wl_display *WAYLAND_DISPLAY = NULL;

static void waylandRegistryGlobal(void *data,struct wl_registry *registry,uint32_t name,const char *interface,uint32_t version) {
    struct window_wayland *window = data;
    
    meow_log(MEOW_LOG_SEVERITY_LEVEL_INFO,"Wayland global registered event (window->windowProperties.name: \"%s\", name: %d, interface: \"%s\", version: %d)",window->windowProperties.name,name,interface,version);
    
    if (!strcmp(interface,"wl_compositor")) {
        window->waylandCompositor = wl_registry_bind(registry,name,&wl_compositor_interface,4);
    }
    
    if (!strcmp(interface,"xdg_wm_base")) {
        window->xdgWindowManagerBase = wl_registry_bind(registry,name,&xdg_wm_base_interface,1);
    }
    
    if (!strcmp(interface,"wl_seat")) {
        window->waylandSeat = wl_registry_bind(registry,name,&wl_seat_interface,1);
    }
}

static void waylandRegistryGlobalRemove(void *data,struct wl_registry *registry,uint32_t name) {
    struct window_wayland *window = data;
    
    meow_log(MEOW_LOG_SEVERITY_LEVEL_INFO,"Wayland global unregistered event (window->windowProperties.name: \"%s\", name: %d, interface: \"%s\", version: %d)",window->windowProperties.name,name);
    
    if (window->waylandCompositor && wl_proxy_get_id((struct wl_proxy *)window->waylandCompositor) == name) {
        wl_compositor_destroy(window->waylandCompositor);
        window->waylandCompositor = NULL;
    }
    
    if (window->xdgWindowManagerBase && wl_proxy_get_id((struct wl_proxy *)window->xdgWindowManagerBase) == name) {
        xdg_wm_base_destroy(window->xdgWindowManagerBase);
        window->xdgWindowManagerBase = NULL;
    }
}

static void xdgWindowManagerBasePing(void *data,struct xdg_wm_base *wm_base,uint32_t serial) {
    xdg_wm_base_pong(wm_base,serial);
}

static void waylandPointerMotion(void *data,struct wl_pointer *pointer,uint32_t time,wl_fixed_t surfaceX,wl_fixed_t surfaceY) {
    struct window_wayland *window = data;
    
    // meow_log(MEOW_LOG_SEVERITY_LEVEL_INFO,"Wayland pointer motion event (window->windowProperties.name: \"%s\", time: %d, surfaceX: %d, surfaceY: %d)",window->windowProperties.name,time,surfaceX,surfaceY);
    
    window->pointer.position = (meow_vector2){
        .x = wl_fixed_to_int(surfaceX),
        .y = wl_fixed_to_int(surfaceY)
    };
}

static void waylandPointerButton(void *data,struct wl_pointer *pointer,uint32_t serial,uint32_t time,uint32_t button,uint32_t state) {
    struct window_wayland *window = data;
    
    meow_log(MEOW_LOG_SEVERITY_LEVEL_INFO,"Wayland pointer button event (window->windowProperties.name: \"%s\", serial: %d, time: %d, button: %d, state: %d)",window->windowProperties.name,serial,time,button,state);
    
    // TODO: Handle button logging
}

static void waylandPointerEnter(void *data,struct wl_pointer *pointer,uint32_t serial,struct wl_surface *surface,wl_fixed_t surfaceX,wl_fixed_t surfaceY) {}

static void waylandPointerLeave(void *data,struct wl_pointer *pointer,uint32_t serial,struct wl_surface *surface) {}

static void waylandKeyboardKeymap(void *data,struct wl_keyboard *keyboard,uint32_t format,int fileDescriptor,uint32_t size) {
    struct window_wayland *window = data;
    
    meow_log(MEOW_LOG_SEVERITY_LEVEL_INFO,"Wayland keyboard keymap event (window->windowProperties.name: \"%s\", format: %d, fileDescriptor: %d, size: %d)",window->windowProperties.name,format,fileDescriptor,size);
    
    if (window->keyboard.keymap) {
        xkb_keymap_unref(window->keyboard.keymap);
        xkb_state_unref(window->keyboard.keymapState);
        xkb_context_unref(window->keyboard.keymapContext);
    }
    
    struct xkb_context *context = xkb_context_new(XKB_CONTEXT_NO_FLAGS);
    
    char *keymapString = mmap(NULL,size,PROT_READ,MAP_SHARED,fileDescriptor,0);
    
    if (keymapString == MAP_FAILED) {
        close(fileDescriptor);
        return;
    }
    
    struct xkb_keymap *keymap = xkb_keymap_new_from_string(
        context,
        keymapString,
        XKB_KEYMAP_FORMAT_TEXT_V1,
        XKB_KEYMAP_COMPILE_NO_FLAGS
    );
    
    munmap(keymapString,size);
    close(fileDescriptor);
    
    if (!keymap) {
        xkb_context_unref(context);
        return;
    }
    
    struct xkb_state *state = xkb_state_new(keymap);
    
    window->keyboard = (struct keyboard_wayland){
        .keymapFormat = format,
        .keymapFileDescriptor = fileDescriptor,
        .keymapSize = size,
        .keymap = keymap,
        .keymapState = state
    };
}

static void waylandKeyboardModifiers(void *data,struct wl_keyboard *keyboard,uint32_t serial,uint32_t depressedModifiers,uint32_t latchedModifiers,uint32_t lockedModifiers,uint32_t group) {
    struct window_wayland *window = data;
    
    meow_log(MEOW_LOG_SEVERITY_LEVEL_INFO,"Wayland keyboard modifiers event (window->windowProperties.name: \"%s\", serial: %d, depressedModifiers: %d, latchedModifiers: %d, lockedModifiers: %d, group: %d)",window->windowProperties.name,serial,depressedModifiers,latchedModifiers,lockedModifiers,group);
    
    xkb_state_update_mask(
        window->keyboard.keymapState,
        depressedModifiers,
        latchedModifiers,
        lockedModifiers,
        0,
        0,
        group
    );
}

static void waylandKeyboardKey(void *data,struct wl_keyboard *keyboard,uint32_t serial,uint32_t time,uint32_t key,uint32_t state) {
    struct window_wayland *window = data;
    
    meow_log(MEOW_LOG_SEVERITY_LEVEL_INFO,"Wayland keyboard key event (window->windowProperties.name: \"%s\", serial: %d, time: %d, key: %d, state: %d)",window->windowProperties.name,serial,time,key,state);
    
    // TODO: Handle key logging
}

static void waylandKeyboardRepeatInformation(void *data,struct wl_keyboard *keyboard,int32_t rate,int32_t delay) {
    struct window_wayland *window = data;
    
    meow_log(MEOW_LOG_SEVERITY_LEVEL_INFO,"Wayland keyboard repetition event (window->windowProperties.name: \"%s\", rate: %d, delay: %d)",window->windowProperties.name,rate,delay);
    
    // TODO: Handle key repetition
}

static void waylandKeyboardEnter(void *data,struct wl_keyboard *wl_keyboard,uint32_t serial,struct wl_surface *surface,struct wl_array *keys) {}

static void waylandKeyboardLeave(void *data,struct wl_keyboard *keyboard,uint32_t serial,struct wl_surface *surface) {}

static void xdgToplevelConfigure(void *data,struct xdg_toplevel *toplevel,int32_t width,int32_t height,struct wl_array *states) {
    struct window_wayland *window = data;
    
    meow_log(MEOW_LOG_SEVERITY_LEVEL_INFO,"XDG toplevel configure event; window resized (window->windowProperties.name: \"%s\", width: %d, height: %d)",window->windowProperties.name,width,height);
    
    if (width > 0 && height > 0) {
        window->windowProperties.size = (meow_vector2){
            .x = width,
            .y = height
        };
        
        if (createSwapchain(&window->vulkanContext,window->windowProperties.size)) {
            window->windowProperties.running = 0;
            return;
        }
    }
}

static void xdgToplevelClose(void *data,struct xdg_toplevel *toplevel) {
    struct window_wayland *window = data;
    
    meow_log(MEOW_LOG_SEVERITY_LEVEL_INFO,"XDG toplevel close event; window closed (window->windowProperties.name: \"%s\")",window->windowProperties.name);
    
    // TODO: Make it proper or watever but for now: If dare's nu recipients to dis event (event system is to be created) den default to destroying da window
    
    destroyWindow_wayland(window);
}

static void xdgSurfaceConfigure(void *data,struct xdg_surface *surface,uint32_t serial) {
    xdg_surface_ack_configure(surface,serial);
}

struct window_wayland *createWindow_wayland(char *name,meow_vector2 size,meow_windowType type) {
    struct window_wayland *window = malloc(sizeof(struct window_wayland));
    memset(window,0,sizeof(struct window_wayland));
    
    window->windowProperties.name = name;
    window->windowProperties.size = size;
    window->windowProperties.type = type;
    window->windowProperties.running = 1;
    
    {
        if (!WAYLAND_DISPLAY) {
            WAYLAND_DISPLAY = wl_display_connect(NULL);
            
            if (!WAYLAND_DISPLAY) {
                meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"Failed to create Wayland display (%d)",__LINE__);
                destroyWindow_wayland(window);
                return NULL;
            }
        }
        
        window->waylandRegistry = wl_display_get_registry(WAYLAND_DISPLAY);
        
        if (!window->waylandRegistry) {
            meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"Failed to get Wayland registry (%d)",__LINE__);
            destroyWindow_wayland(window);
            return NULL;
        }
        
        window->waylandRegistryListener = (struct wl_registry_listener){
            .global = waylandRegistryGlobal,
            .global_remove = waylandRegistryGlobalRemove
        };
        
        if (wl_registry_add_listener(window->waylandRegistry,&window->waylandRegistryListener,window)) {
            meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"Failed to add Wayland registry listener (%d)",__LINE__);
            destroyWindow_wayland(window);
            return NULL;
        }
        
        if (wl_display_roundtrip(WAYLAND_DISPLAY) == -1) {
            meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"Failed to capture Wayland globals (%d)",__LINE__);
            destroyWindow_wayland(window);
            return NULL;
        }
        
        if (!window->waylandCompositor) {
            meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"Failed to bind Wayland compositor global (%d)",__LINE__);
            destroyWindow_wayland(window);
            return NULL;
        }
        
        if (!window->xdgWindowManagerBase) {
            meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"Failed to bind XDG window manager base global (%d)",__LINE__);
            destroyWindow_wayland(window);
            return NULL;
        }
        
        if (!window->waylandSeat) {
            meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"Failed to bind Wayland seat global (%d)",__LINE__);
            destroyWindow_wayland(window);
            return NULL;
        }
        
        window->xdgWindowManagerBaseListener = (struct xdg_wm_base_listener){
            .ping = xdgWindowManagerBasePing
        };
        
        xdg_wm_base_add_listener(window->xdgWindowManagerBase,&window->xdgWindowManagerBaseListener,NULL);
        
        window->waylandPointerListener = (struct wl_pointer_listener){
            .motion = waylandPointerMotion,
            .button = waylandPointerButton,
            .enter = waylandPointerEnter,
            .leave = waylandPointerLeave
        };
        
        wl_pointer_add_listener(wl_seat_get_pointer(window->waylandSeat),&window->waylandPointerListener,window);
        
        window->waylandKeyboardListener = (struct wl_keyboard_listener){
            .keymap = waylandKeyboardKeymap,
            .modifiers = waylandKeyboardModifiers,
            .key = waylandKeyboardKey,
            .repeat_info = waylandKeyboardRepeatInformation,
            .enter = waylandKeyboardEnter,
            .leave = waylandKeyboardLeave
        };
        
        wl_keyboard_add_listener(wl_seat_get_keyboard(window->waylandSeat),&window->waylandKeyboardListener,window);
        
        window->waylandSurface = wl_compositor_create_surface(window->waylandCompositor);
        
        if (!window->waylandSurface) {
            meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"Failed to create Wayland surface (%d)",__LINE__);
            destroyWindow_wayland(window);
            return NULL;
        }
        
        window->xdgSurface = xdg_wm_base_get_xdg_surface(window->xdgWindowManagerBase,window->waylandSurface);
        
        if (!window->xdgSurface) {
            meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"Failed to get XDG surface (%d)",__LINE__);
            destroyWindow_wayland(window);
            return NULL;
        }
        
        if (window->windowProperties.type == MEOW_WINDOW_TYPE_TOPLEVEL) {
            window->xdgToplevelWindow = xdg_surface_get_toplevel(window->xdgSurface);
            
            if (!window->xdgToplevelWindow) {
                meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"Failed to get XDG toplevel window (%d)",__LINE__);
                destroyWindow_wayland(window);
                return NULL;
            }
            
            window->xdgToplevelListener = (struct xdg_toplevel_listener){
                .configure = xdgToplevelConfigure,
                .close = xdgToplevelClose
            };
            
            if (xdg_toplevel_add_listener(window->xdgToplevelWindow,&window->xdgToplevelListener,window)) {
                meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"Failed to add XDG toplevel listener (%d)",__LINE__);
                destroyWindow_wayland(window);
                return NULL;
            }
            
            xdg_toplevel_set_title(window->xdgToplevelWindow,window->windowProperties.name);
        }
        
        xdg_surface_set_window_geometry(window->xdgSurface,0,0,window->windowProperties.size.x,window->windowProperties.size.y);
        
        window->xdgSurfaceListener = (struct xdg_surface_listener){
            .configure = xdgSurfaceConfigure
        };
        
        if (xdg_surface_add_listener(window->xdgSurface,&window->xdgSurfaceListener,window)) {
            meow_log(MEOW_LOG_SEVERITY_LEVEL_ERROR,"Failed to add XDG surface listener (%d)",__LINE__);
            destroyWindow_wayland(window);
            return NULL;
        }
        
        wl_surface_commit(window->waylandSurface);
    }
    
    if (initializeVulkanContext(&window->vulkanContext,window->waylandSurface,WAYLAND_DISPLAY,NULL,window->windowProperties.size)) {
        return NULL;
    }
    
    window->vulkanContext.currentFrame = 0;
    
    return window;
}

_Bool windowIsRunning_wayland(struct window_wayland *window) {
    return window ? window->windowProperties.running : 0;
}

void dispatchEvents_wayland() {
    wl_display_dispatch(WAYLAND_DISPLAY);
}

void renderWindowFrame_wayland(struct window_wayland *window) {
    if (window) {
        if (renderVulkanContextFrame(&window->vulkanContext)) {
            window->windowProperties.running = 0;
        }
    }
}

void destroyWindow_wayland(struct window_wayland *window) {
    deinitializeVulkanContext(&window->vulkanContext);
    
    if (window->xdgToplevelWindow) {
        xdg_toplevel_destroy(window->xdgToplevelWindow);
    }
    
    if (window->xdgSurface) {
        xdg_surface_destroy(window->xdgSurface);
    }
    
    if (window->waylandSurface) {
        wl_surface_destroy(window->waylandSurface);
    }
}

#endif
