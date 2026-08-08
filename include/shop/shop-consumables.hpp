#pragma once
#include <string>
#include "raylib.h"
#include "consumables.hpp"
#include "variables.hpp"

struct ShopConsumableEntry {
    std::string name;
    std::string description;
    Color color;
    ConsumableEffectType effectType;
    void (*useFn)(Consumable&);
    int sellValue;
    int price;
    int maxTargets = 1;
    int rarityweight = 100;
};

inline std::vector<ShopConsumableEntry> consumableShopPool = {
    { "Fire Sale", "Every daemon in your hand adds its full sell value to your balance", Config::COLOR_UI_AMBER, ConsumableEffectType::INSTANT, firesale, 60, 300 },
    { "Decrypt", "Select an encrypted node on the map to reveal it", Config::MAGENTA_DAEMON, ConsumableEffectType::BOARD_TARGET, UseDecryptNode, 150, 450 },
    { "Overclock Pin", "Set up to two pins' modifiers to a flat boosted payout. Incompatible with Volatile", Config::COLOR_UI_GREEN, ConsumableEffectType::BOARD_TARGET, SetNodeModifierBoost, 130, 320, 2 },
    { "Volatile Pin", "Set up to two pins' modifiers to an unstable, random payout. Incompatible with Overclock", Config::COLOR_UI_AMBER, ConsumableEffectType::BOARD_TARGET, SetNodeModifierGlitch, 130, 320, 2 },
    { "Clone Pin", "Set a pin's modifier to split and divide score between two balls", Config::MAGENTA_DAEMON, ConsumableEffectType::BOARD_TARGET, SetNodeModifierClone, 150, 320, 1,25},
    { "Port Overclock", "Select a basket to permanently boost its multiplier by 1.5x", Config::COLOR_UI_AMBER, ConsumableEffectType::BOARD_TARGET, boostbasketmult, 200, 500 ,1,25},
};