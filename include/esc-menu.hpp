#pragma once
#include "variables.hpp"
#include "animation-timer.hpp"

enum esc_menu_states{
    MAIN,
    SETTINGZ,
    STATS
};

inline esc_menu_states esc_menu_state = MAIN;
inline Timer exitanimtimer = {0};
struct StatEntry {
    std::string label;
    std::function<std::string()> getValue;
};


enum class SettingType {TOGGLE,SLIDER,NOTCHED_SLIDER};

struct SettingEntry {
    std::string label;
    SettingType type;
    bool* boolValue = nullptr;
    float* floatValue = nullptr;
    float minValue = 0.0f;
    float maxValue = 1.0f;
};


void drawescmenu();