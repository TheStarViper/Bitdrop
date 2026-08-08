#include "colortools.hpp"

Color ScaleAlpha(Color c, float mul) {
    c.a = (unsigned char)(c.a * mul);
    return c;
}

Color DarkenColor(Color c, float factor) {
    return Color{
        (unsigned char)(c.r * (1.0f - factor)),
        (unsigned char)(c.g * (1.0f - factor)),
        (unsigned char)(c.b * (1.0f - factor)),
        c.a
    };
}

void DrawSpriteWithHueShader(Texture2D texture, Rectangle srcRect, Rectangle destRect, Color baseColor, Shader shader, int hueLoc) {
    Vector3 hsv = ColorToHSV(baseColor);
    float hue = 0.2f + hsv.x / 360.0f;
    SetShaderValue(shader, hueLoc, &hue, SHADER_UNIFORM_FLOAT);

    BeginShaderMode(shader);
    DrawTexturePro(texture, srcRect, destRect, { 0, 0 }, 0.0f, WHITE);
    EndShaderMode();
}