#pragma once
#include "raylib.h"

Color ScaleAlpha(Color c, float mul);
Color DarkenColor(Color c, float factor);
void DrawSpriteWithHueShader(Texture2D texture, Rectangle srcRect, Rectangle destRect, Color baseColor, Shader shader, int hueLoc);