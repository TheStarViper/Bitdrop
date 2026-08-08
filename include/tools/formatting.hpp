#pragma once
#include <string>
#include "raylib.h"
#include <vector>

std::string FormatByteSize(long double bytes);
std::string formatWithSpaces(long long int num);
float GetFittingFontSize(const char* text, float maxFontSize, float maxWidth, Font font = GetFontDefault());
static std::vector<std::string> WrapText(const std::string& text, Font font, float fontSize, float maxWidth);
std::string ToUpperString(std::string text);