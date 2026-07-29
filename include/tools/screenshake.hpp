#pragma once
#include "raylib.h"

void screenshake(float intensity, float duration);
void UpdateScreenShake(float dt);
Vector2 GetScreenShakeOffset();
