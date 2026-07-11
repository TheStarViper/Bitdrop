#pragma once
#include "raylib.h"

enum class ButtonType {
    ArrowUp,
    ArrowDown,
    TextGeneric
};

bool DrawButton(Rectangle rect, ButtonType type, unsigned char alpha, Color bgNormal, Color bgHover, Color borderCol, Color contentCol, const char* text = nullptr, int fontSize = 11);