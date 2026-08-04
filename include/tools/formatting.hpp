#pragma once
#include <string>
#include "raylib.h"

std::string FormatByteSize(long double bytes);
std::string formatWithSpaces(long long int num);
float GetFittingFontSize(const char* text, float maxFontSize, float maxWidth, Font font = GetFontDefault());