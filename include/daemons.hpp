#pragma once
#include <functional>
#include "raylib.h"
#include <string>
#include "variables.hpp"
#include "easing_functions.hpp"
#include "shop.hpp"

class Probe;
class Daemon {
public:
    using DaemonAction = void(*)(Daemon&,Probe&);
private:
    std::string name;
    std::string description;
    int level;
    int max_level;
    int fillPct;
    Color accentColor;
    int overclocked;
    float expansionTimer = 0.0f;
    int sellval;
    DaemonAction actionCallback;

public:
    std::function<void(class GameEngine&, float)> systemPatch;
    float x = 830.0f;
    float tempy;
    float y;
    float width = 420.0f;
    float height = 76.0f;
    std::string status;
    int slot;
    int price;
    const IconGrid* iconMatrix;
    DaemonTriggersType triggertype;

    Daemon(std::string daemonName, 
           std::string stat, 
           std::string desc, 
           Color identityColor,
           int maxlvl,
           int itemCost,
           const IconGrid* icon,
           DaemonTriggersType trigger = PASSIVE,
           DaemonAction action = nullptr,
           int sellvals = 0,
           int slott = -1) {
        slot = slott;
        sellval = sellvals;
        y = Config::Daemon_Y_Buffer + ((slot - 1) % 5) * (height + Config::Daemon_Slot_Spacing);
        name = daemonName;
        status = stat;
        price = itemCost;
        description = desc;
        accentColor = identityColor;
        level = 1;
        max_level = maxlvl;
        fillPct = 0;
        triggertype = trigger;
        actionCallback = action;
        overclocked = 0;
        systemPatch = nullptr;
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

    void updateYPosition() {
        y = Config::Daemon_Y_Buffer + ((slot - 1) % 5) * (height + Config::Daemon_Slot_Spacing);
    }

    int getsellval() const {
        return price/2 + (10 * static_cast<int>(std::pow(overclocked, 3)));
    }

    void TriggerAction(Probe& probe) {
        if (actionCallback != nullptr) {
            actionCallback(*this,probe);
        }
    }
    
    float GetExpansion() const { return Easings::EaseInOutQuart(expansionTimer); }
    //float GetExpansion() const { return expansionTimer; } //no easings just linear
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
void DrawCyberpunkDaemonSlot(const Daemon& d, Vector2 mousePos, bool isSelected, int daemonidx, int* selectedDaemonIndex);
void initdaemons();
void ProcessLineFades(GameEngine& eng);
void DrawFadingLines(const GameEngine& eng);