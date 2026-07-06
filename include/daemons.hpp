#pragma once
#include <functional>
#include "raylib.h"
#include <string>
#include "variables.hpp"
#include "easing_functions.hpp"

class Daemon {
private:
    std::string name;
    std::string description;
    int level;
    int max_level;
    int experience;
    int maxExperience;
    int fillPct;
    Color accentColor;
    int overclocked;
    float expansionTimer = 0.0f;
    float slotYStart = 15.0f;
    float slotSpacing = 10.0f;

public:
    std::function<void(class GameEngine&, float)> systemPatch;
    float x = 830.0f;
    float tempy;
    float y;
    float width = 420.0f;
    float height = 76.0f;
    std::string status;
    int slot;

    Daemon(int slott, 
           std::string daemonName, 
           std::string stat, 
           std::string desc, 
           Color identityColor,
           int maxlvl) {
        slot = slott;
        y = slotYStart + (slot - 1) * (height + slotSpacing);
        name = daemonName;
        status = stat;
        description = desc;
        accentColor = identityColor;
        level = 1;
        max_level = maxlvl;
        experience = 0;
        maxExperience = 100;
        fillPct = 0;
        overclocked = 0;
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

    void UpdateExpansion(float dt, bool isSelected) {
        if (isSelected) {
            expansionTimer += dt * 8.0f;
            if (expansionTimer > 1.0f) expansionTimer = 1.0f;
        } else {
            expansionTimer -= dt * 8.0f;
            if (expansionTimer < 0.0f) expansionTimer = 0.0f;
        }
    }

    float GetExpansion() const { return Easings::EaseInOutQuart(expansionTimer); }
    std::string GetName() const { return name; }
    std::string GetDesc() const { return description; }
    int GetLevel() const { return level; }
    int getmaxlevel() const { return max_level; }
    int GetFillPct() const { return fillPct; }
    void SetFillPct(int pct) { fillPct = pct; }
    Color GetColor() const { return accentColor; }
    bool IsOverclocked() const { return overclocked > 0; }
    void SetOverclock(int overclocklvl) { overclocked = overclocklvl; } //debugging
    int getoverclocklvl () const { return overclocked; }
    //me realising you can just use variables instead of a function to return them in a class whoopsies but i diont feel like changing it now
};

void PrepDrawCyberpunkDaemonSlots();
void DrawCyberpunkDaemonSlot(const Daemon& d, Vector2 mousePos, bool isSelected);
void initdaemons();