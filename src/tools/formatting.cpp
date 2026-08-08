#include "formatting.hpp"
#include <string>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <vector>
#include "raylib.h"

std::string FormatByteSize(long double bytes) {
    if (bytes < 1) return "0 B";
    if (bytes < 1024.0) return std::to_string(bytes)+" B";
    const char* suffixes[] = {"MB", "GB", "TB", "PB", "EB", "ZB", "YB", "RB", "QB" };
    int i = 0;
    long double size = bytes / 1024.0;
    while (size >= 1024.0 && i < 8) {
        size /= 1024.0;
        i++;
    }
    std::stringstream stream;
    if (i == 8 && size >= 1024.0) {
        stream << std::scientific << std::setprecision(2) << size << " " << suffixes[i];
    } else {
        stream << std::fixed << std::setprecision(2) << size << " " << suffixes[i];
    }
    return stream.str();
}

std::string formatWithSpaces(long long int num) {
    std::string str = std::to_string(num);
    std::string result = "";
    int count = 0;

    for (int i = str.length() - 1; i >= 0; i--) {
        if (count == 3) {
            result += " ";
            count = 0;
        }
        result += str[i];
        count++;
    }

    std::reverse(result.begin(), result.end());
    return result;
}

float GetFittingFontSize(const char* text, float maxFontSize, float maxWidth, Font font) {
    float fontSize = maxFontSize;
    Vector2 textSize = MeasureTextEx(font, text, fontSize, 1.0f);

    if (textSize.x > maxWidth) {
        fontSize = (maxWidth / textSize.x) * maxFontSize;
    }
    return fontSize;
}

static std::vector<std::string> WrapText(const std::string& text, Font font, float fontSize, float maxWidth) {
    std::vector<std::string> lines;
    std::string currentLine;
    std::string word;
    std::stringstream ss(text);

    while (ss >> word) {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        Vector2 size = MeasureTextEx(font, testLine.c_str(), fontSize, 1.0f);
        if (size.x > maxWidth) {
            if (!currentLine.empty()) lines.push_back(currentLine);
            currentLine = word;
        } else {
            currentLine = testLine;
        }
    }
    if (!currentLine.empty()) lines.push_back(currentLine);
    return lines;
}

std::string ToUpperString(std::string text) {
    for (auto& ch : text) {
        ch = (char)toupper((unsigned char)ch);
    }
    return text;
}