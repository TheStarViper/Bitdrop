#pragma once
#include <string>

struct TutHint{
    std::string id;
    std::string content;
    bool shown = false;
    int x;
    int y;
    int width;
};

void triggerhint(const std::string& id, int x, int y, int width=300);
void drawhint();