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
