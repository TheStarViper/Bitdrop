#pragma once
#include "raylib.h"
#include "variables.hpp"
#include "consumables.hpp"
#include "variables.hpp"
#include <string>
#include <sstream>

struct ModifierDef {
    ModifierType type;
    std::string name;
    Color color;
    int maxLevel;
    std::vector<ModifierType> incompatibleWith;
    std::string (*getTooltip)(int level);
    void (*applyScoreEffect)(int level, long double& byteBump, float& bufferRateBump);
    bool triggersClone;
    void (*useFn)(Consumable&);
};

std::string BoostTooltip(int level);
void BoostScoreEffect(int level, long double& byteBump, float&);
std::string GlitchTooltip(int);
void GlitchScoreEffect(int level, long double& byteBump, float& bufferRateBump);
std::string CloneTooltip(int);
void CloneScoreEffect(int, long double&, float&);
int GetModifierMaxLevel(ModifierType type);
const std::vector<ModifierDef>& GetModifierRegistry();
const ModifierDef* GetModifierDef(ModifierType type);
int GetModifierLevel(const Node& node, ModifierType type);
bool HasModifier(const Node& node, ModifierType type);
void RemoveIncompatibleModifiers(std::vector<ActiveModifier>& existing, ModifierType incoming);
void ApplyOrLevelModifier(Node* target, ModifierType type);
ModifierType GetModifierTypeForConsumableFn(ConsumableUseFn fn);
void ApplyModifierToPendingTargets(ModifierType type);
Color GetNodePinColor(const Node& node);