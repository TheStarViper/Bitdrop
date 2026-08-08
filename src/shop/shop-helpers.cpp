#include "shop-helpers.hpp"


Texture2D GetShopItemSprite(bool pressed) {
    static Texture2D tex;
    static Texture2D texpressed;
    static bool loaded = false;
    if (!loaded) {
        tex = LoadTexture("assets/shop-item.png");
        texpressed = LoadTexture("assets/shop-item-pressed.png");
        loaded = true;
    }
    return !pressed ? tex : texpressed;
}

Texture2D GetConsumableSlotSprite() {
    static Texture2D tex;
    static bool loaded = false;
    if (!loaded) {
        tex = LoadTexture("assets/consumable-shop-item.png");
        loaded = true;
    }
    return tex;
}
