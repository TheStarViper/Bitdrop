#pragma once
#include <functional>
#include "raylib.h"
#include <string>
#include "variables.hpp"


class Daemon {
private:
    std::string name;
    std::string description;
    int level;
    int experience;
    int maxExperience;
    int fillPct;
    Color accentColor;
    bool activeGlitch;

public:
    std::function<void(class GameEngine&, float)> systemPatch;
    float x, y, width, height;
    std::string status;
    Daemon(float posX, float posY, float w, float h, std::string daemonName, std::string stat, std::string desc, Color identityColor) {
        x = posX;
        y = posY;
        width = w;
        height = h;
        name = daemonName;
        status = stat;
        description = desc;
        accentColor = identityColor;
        level = 1;
        experience = 0;
        maxExperience = 100;
        fillPct = 0;
        activeGlitch = false;
        systemPatch = nullptr;
    }

    void GainXP(int amount) {
        if (name == "EMPTY_SLOT_DECK") return;
        experience += amount;
        if (experience >= maxExperience) {
            level++;
            experience -= maxExperience;
            maxExperience = (int)(maxExperience * 1.5f); 
            fillPct = 0;
        }
    }

    void ExecuteRoutine(class GameEngine& eng, float dt) {
        if (systemPatch != nullptr) {
            systemPatch(eng, dt);
        }
    }

    std::string GetName() const { return name; }
    std::string GetDesc() const { return description; }
    int GetLevel() const { return level; }
    int GetFillPct() const { return fillPct; }
    void SetFillPct(int pct) { fillPct = pct; }
    Color GetColor() const { return accentColor; }
    bool IsGlitched() const { return activeGlitch; }
    void ToggleGlitch(bool state) { activeGlitch = state; }
};