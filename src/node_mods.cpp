#include "node_mods.hpp"
#include <algorithm>

std::string BoostTooltip(int level) {
    float mult = 2.5f + (level - 1) * 1.0f;
    std::ostringstream oss;
    oss << "OVERCLOCK" << mult;
    return oss.str();
}

void BoostScoreEffect(int level, long double& byteBump, float&) {
    byteBump *= (2.5 + (level - 1) * 1.0);
}

std::string GlitchTooltip(int) {
    return "VOLATILE";
}

void GlitchScoreEffect(int level, long double& byteBump, float& bufferRateBump) {
    byteBump *= ((float)GetRandomValue(5, 50 + (level - 1) * 25) * 0.2f);
    bufferRateBump = 0.35f;
}

std::string CloneTooltip(int) {
    return "CLONE";
}

void CloneScoreEffect(int, long double&, float&) {
}

const std::vector<ModifierDef>& GetModifierRegistry() {
    static const std::vector<ModifierDef> registry = {
        { MOD_BOOST, "BOOST", Config::COLOR_UI_GREEN, 2, { MOD_GLITCH }, BoostTooltip, BoostScoreEffect, false, SetNodeModifierBoost },
        { MOD_GLITCH, "GLITCH", (Color){ 255, 50, 140, 255 }, 3, { MOD_BOOST }, GlitchTooltip, GlitchScoreEffect, false, SetNodeModifierGlitch },
        { MOD_CLONE, "CLONE", (Color){ 200, 50, 255, 255 }, 2, {}, CloneTooltip, CloneScoreEffect, true, SetNodeModifierClone }
    };
    return registry;
}

const ModifierDef* GetModifierDef(ModifierType type) {
    for (const auto& def : GetModifierRegistry()) {
        if (def.type == type) return &def;
    }
    return nullptr;
}

int GetModifierLevel(const Node& node, ModifierType type) {
    for (const auto& m : node.modifiers) {
        if (m.type == type) return m.level;
    }
    return 0;
}

bool HasModifier(const Node& node, ModifierType type) {
    return GetModifierLevel(node, type) > 0;
}

int GetModifierMaxLevel(ModifierType type) {
    const ModifierDef* def = GetModifierDef(type);
    return def ? def->maxLevel : 1;
}

void RemoveIncompatibleModifiers(std::vector<ActiveModifier>& existing, ModifierType incoming) {
    const ModifierDef* def = GetModifierDef(incoming);
    if (!def) return;
    for (ModifierType conflict : def->incompatibleWith) {
        existing.erase(std::remove_if(existing.begin(), existing.end(),
            [conflict](const ActiveModifier& m) { return m.type == conflict; }), existing.end());
    }
}

void ApplyOrLevelModifier(Node* target, ModifierType type) {
    if (!target) return;
    RemoveIncompatibleModifiers(target->modifiers, type);

    for (auto& m : target->modifiers) {
        if (m.type == type) {
            if (m.level < GetModifierMaxLevel(type)) m.level++;
            target->pulseAnimTimer = 1.0f;
            return;
        }
    }
    target->modifiers.push_back({ type, 1 });
    target->pulseAnimTimer = 1.0f;
}

ModifierType GetModifierTypeForConsumableFn(ConsumableUseFn fn) {
    for (const auto& def : GetModifierRegistry()) {
        if (def.useFn == fn) return def.type;
    }
    return MOD_NONE;
}

void ApplyModifierToPendingTargets(ModifierType type) {
    int count = GetPendingConsumableTargetCount();
    for (int i = 0; i < count; i++) {
        ApplyOrLevelModifier(static_cast<Node*>(GetPendingConsumableTarget(i)), type);
    }
    const ModifierDef* def = GetModifierDef(type);
    engine.calculationLog = (def ? def->name : "UNKNOWN") + " MODIFIER INJECTED";
}

Color GetNodePinColor(const Node& node) {
    for (const auto& def : GetModifierRegistry()) {
        if (HasModifier(node, def.type)) return def.color;
    }
    return Config::COLOR_NODE;
}

void DrawNodeIndicators(const Node& node) {
    if (node.modifiers.empty()) return;

    int count = (int)node.modifiers.size();
    float anglePerMod = 360.0f / count;
    float ringInner = node.currentRadius + 3.0f;
    float ringOuter = node.currentRadius + 6.0f;

    for (int i = 0; i < count; i++) {
        const auto& mod = node.modifiers[i];
        const ModifierDef* def = GetModifierDef(mod.type);
        if (!def) continue;

        float startAngle = i * anglePerMod;
        float endAngle = startAngle + anglePerMod - 4.0f;

        DrawRing(node.position, ringInner, ringOuter, startAngle, endAngle, 12, def->color);

        if (def->maxLevel > 1) {
            float midAngle = ((startAngle + endAngle) / 2.0f) * DEG2RAD;
            Vector2 pipCenter = {
                node.position.x + cosf(midAngle) * (ringOuter + 5.0f),
                node.position.y + sinf(midAngle) * (ringOuter + 5.0f)
            };
            for (int lvl = 0; lvl < mod.level; lvl++) {
                float offset = (lvl - (mod.level - 1) / 2.0f) * 4.0f;
                DrawCircleV({ pipCenter.x + offset, pipCenter.y }, 1.5f, def->color);
            }
        }
    }
}
