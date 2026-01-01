#pragma once

#include <meow_utilities/include/meow_math.h>
#include <meow_window/include/window.h>

typedef struct {
    meow_vector4 color;
} meow_style;

void meow_drawRectangle(meow_window *window,meow_vector2 position,meow_vector2 size,meow_style style);
