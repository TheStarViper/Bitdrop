#pragma once
#include "raylib.h"
#include <string>
#include <vector>

enum class ConsumableEffectType {
    INSTANT,
    BOARD_TARGET
};

struct Consumable {
    std::string name;
    std::string description;
    Color color;
    int sellValue;
    ConsumableEffectType effectType;
    void (*useFn)(Consumable&);
    Texture2D* icon = nullptr;

    int slot = 0;
    float x = 0.0f;
    float y = 0.0f;
    float width = 78.0f;
    float height = 100.0f;

    bool isHovered = false;
    bool wasHovered = false;
    float expansion = 0.0f;

    Consumable(std::string n, std::string desc, Color c, int sellVal, ConsumableEffectType type,
               void (*fn)(Consumable&), Texture2D* ic = nullptr)
        : name(std::move(n)), description(std::move(desc)), color(c), sellValue(sellVal),
          effectType(type), useFn(fn), icon(ic) {}

    void updatePosition();
};

struct ConsumableInventory {
    std::vector<Consumable> consumables;
};

extern ConsumableInventory activeconsumableinfo;

void PrepDrawConsumableSlots();
void DrawConsumableEmptySlot(int slotIndex);
int DrawConsumableSlot(Consumable& c, Vector2 mousePos, int idx, bool isSelected);

bool IsConsumablePending();
int GetPendingConsumableIndex();
void CancelPendingConsumable();
void ResolvePendingConsumable();

void UseRerollCharge(Consumable& self);
void UseOverclockBooster(Consumable& self);
void UseBoardWipeCharge(Consumable& self);