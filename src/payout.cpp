#include "variables.hpp"
#include <algorithm>
#include "payout.hpp"

void SpawnEnergyOrb(Vector2 from, Vector2 to, int value, float startDelay) {
    EnergyOrbInstance orb;
    orb.startPos = from;
    orb.endPos = to;
    orb.value = value;
    orb.timer = -startDelay;
    activeOrbs.push_back(orb);
}

void UpdateEnergyOrbs() {
    for (auto& orb : activeOrbs) {
        if (orb.bursting) {
            orb.burstTimer += GetFrameTime();
            continue;
        }

        orb.timer += GetFrameTime();
        if (orb.timer < 0.0f) continue;

        orb.travelling = true;
        float t = Clamp(orb.timer / orb.duration, 0.0f, 1.0f);
        float eased = Easings::EaseInOutQuad(t);

        orb.trailSpawnTimer += GetFrameTime();
        if (orb.trailSpawnTimer >= 0.015f) {
            orb.trailSpawnTimer = 0.0f;
            Vector2 pos = Vector2Lerp(orb.startPos, orb.endPos, eased);
            pos.x += GetRandomValue(-3, 3);
            pos.y += GetRandomValue(-3, 3);
            orb.trail.push_back({ pos, 1.0f });
        }

        for (auto& p : orb.trail) p.life -= GetFrameTime() * 3.0f;
        orb.trail.erase(
            std::remove_if(orb.trail.begin(), orb.trail.end(),
                [](const EnergyTrailPoint& p){ return p.life <= 0.0f; }),
            orb.trail.end()
        );

        if (t >= 1.0f) {
            orb.travelling = false;
            orb.bursting = true;
            orb.burstTimer = 0.0f;
            gamestate.balance += orb.value;
        }
    }

    activeOrbs.erase(
        std::remove_if(activeOrbs.begin(), activeOrbs.end(),
            [](const EnergyOrbInstance& o) { return o.bursting && o.burstTimer >= o.burstDuration; }),
        activeOrbs.end()
    );
}

void DrawEnergyOrbs() {
    for (const auto& orb : activeOrbs) {
        for (const auto& p : orb.trail) {
            Color trailCol = Fade((Color){ 0, 255, 140, 255 }, p.life * 0.6f);
            float size = 4.0f * p.life;
            DrawRectangle((int)(p.pos.x - size/2), (int)(p.pos.y - size/2), (int)size, (int)size, trailCol);
        }

        if (orb.travelling) {
            float t = Clamp(orb.timer / orb.duration, 0.0f, 1.0f);
            float eased = Easings::EaseInOutQuad(t);
            Vector2 pos = Vector2Lerp(orb.startPos, orb.endPos, eased);
            pos.x += GetRandomValue(-1, 1);
            pos.y += GetRandomValue(-1, 1);

            float pulse = 5.0f + sinf(GetTime() * 20.0f) * 1.5f;
            DrawCircleV(pos, pulse + 3.0f, Fade((Color){ 0, 255, 140, 255 }, 0.25f));
            DrawCircleV(pos, pulse, (Color){ 0, 255, 140, 255 });
            DrawCircleLinesV(pos, pulse + 2.0f, Fade(WHITE, 0.6f));
        }

        if (orb.bursting) {
            float t = Clamp(orb.burstTimer / orb.burstDuration, 0.0f, 1.0f);
            float intensity = sinf(t * PI);
            Vector2 center = orb.endPos;

            int barCount = (int)(intensity * 6.0f);
            for (int i = 0; i < barCount; i++) {
                int bx = (int)center.x + GetRandomValue(-40, 40);
                int by = (int)center.y + GetRandomValue(-15, 15);
                DrawRectangle(bx, by, GetRandomValue(10, 50), GetRandomValue(2, 5),
                    Fade((Color){ 0, 255, 140, 255 }, intensity));
            }

            DrawCircleV(center, 10.0f * intensity, Fade(RED, 0.4f * intensity));
            DrawCircleV({ center.x - 3, center.y }, 10.0f * intensity, Fade((Color){0,180,255,255}, 0.4f * intensity));
            DrawCircleV(center, 8.0f * intensity, Fade(WHITE, 0.5f * intensity));

            if (t < 0.4f) {
                DrawRectangle(Config::walletX, Config::walletY, 420, 65, Fade((Color){0,255,140,255}, (0.4f - t) * 0.5f));
            }
        }
    }
}