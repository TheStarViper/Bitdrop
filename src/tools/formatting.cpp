#include "formatting.hpp"
#include <string>
#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

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