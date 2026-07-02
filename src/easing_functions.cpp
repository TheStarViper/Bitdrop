#include <cmath>
#include "easing_functions.hpp"

//https://easings.net/
constexpr float pi = 3.14159265358979;
namespace Easings{

    float EaseLinear(float t) { return t; }

    float EaseInSine(float t) { return 1.0f - std::cos((t * pi) / 2.0f); }
    float EaseOutSine(float t) { return std::sin((t * pi) / 2.0f); }
    float EaseInOutSine(float t) { return -(std::cos(pi * t) - 1.0f) / 2.0f; }

    float EaseInQuad(float t) { return t * t; }
    float EaseOutQuad(float t) { return 1.0f - (1.0f - t) * (1.0f - t); }
    float EaseInOutQuad(float t) { return t < 0.5f ? 2.0f * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 2.0f) / 2.0f; }

    float EaseInCubic(float t) { return t * t * t; }
    float EaseOutCubic(float t) { return 1.0f - std::pow(1.0f - t, 3.0f); }
    float EaseInOutCubic(float t) { return t < 0.5f ? 4.0f * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f; }

    float EaseInQuart(float t) { return t * t * t * t; }
    float EaseOutQuart(float t) { return 1.0f - std::pow(1.0f - t, 4.0f); }
    float EaseInOutQuart(float t) { return t < 0.5f ? 8.0f * t * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 4.0f) / 2.0f; }

    float EaseInQuint(float t) { return t * t * t * t * t; }
    float EaseOutQuint(float t) { return 1.0f - std::pow(1.0f - t, 5.0f); }
    float EaseInOutQuint(float t) { return t < 0.5f ? 16.0f * t * t * t * t * t : 1.0f - std::pow(-2.0f * t + 2.0f, 5.0f) / 2.0f; }

    float EaseInExpo(float t) { return t == 0.0f ? 0.0f : std::pow(2.0f, 10.0f * t - 10.0f); }
    float EaseOutExpo(float t) { return t == 1.0f ? 1.0f : 1.0f - std::pow(2.0f, -10.0f * t); }
    float EaseInOutExpo(float t) {
        if (t == 0.0f) return 0.0f;
        if (t == 1.0f) return 1.0f;
        return t < 0.5f ? std::pow(2.0f, 20.0f * t - 10.0f) / 2.0f : (2.0f - std::pow(2.0f, -20.0f * t + 10.0f)) / 2.0f;
    }

    float EaseInCirc(float t) { return 1.0f - std::sqrt(1.0f - std::pow(t, 2.0f)); }
    float EaseOutCirc(float t) { return std::sqrt(1.0f - std::pow(t - 1.0f, 2.0f)); }
    float EaseInOutCirc(float t) {
        return t < 0.5f ? (1.0f - std::sqrt(1.0f - std::pow(2.0f * t, 2.0f))) / 2.0f
                        : (std::sqrt(1.0f - std::pow(-2.0f * t + 2.0f, 2.0f)) + 1.0f) / 2.0f;
    }

    float EaseInBack(float t) {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1.0f;
        return c3 * t * t * t - c1 * t * t;
    }
    float EaseOutBack(float t) {
        const float c1 = 1.70158f;
        const float c3 = c1 + 1.0f;
        return 1.0f + c3 * std::pow(t - 1.0f, 3.0f) + c1 * std::pow(t - 1.0f, 2.0f);
    }
    float EaseInOutBack(float t) {
        const float c1 = 1.70158f;
        const float c2 = c1 * 1.525f;
        return t < 0.5f ? (std::pow(2.0f * t, 2.0f) * ((c2 + 1.0f) * 2.0f * t - c2)) / 2.0f
                        : (std::pow(2.0f * t - 2.0f, 2.0f) * ((c2 + 1.0f) * (t * 2.0f - 2.0f) + c2) + 2.0f) / 2.0f;
    }

    float EaseInElastic(float t) {
        const float c4 = (2.0f * pi) / 3.0f;
        if (t == 0.0f) return 0.0f;
        if (t == 1.0f) return 1.0f;
        return -std::pow(2.0f, 10.0f * t - 10.0f) * std::sin((t * 10.0f - 10.75f) * c4);
    }
    float EaseOutElastic(float t) {
        const float c4 = (2.0f * pi) / 3.0f;
        if (t == 0.0f) return 0.0f;
        if (t == 1.0f) return 1.0f;
        return std::pow(2.0f, -10.0f * t) * std::sin((t * 10.0f - 0.75f) * c4) + 1.0f;
    }
    float EaseInOutElastic(float t) {
        const float c5 = (2.0f * pi) / 4.5f;
        if (t == 0.0f) return 0.0f;
        if (t == 1.0f) return 1.0f;
        return t < 0.5f ? -(std::pow(2.0f, 20.0f * t - 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) / 2.0f
                        : (std::pow(2.0f, -20.0f * t + 10.0f) * std::sin((20.0f * t - 11.125f) * c5)) / 2.0f + 1.0f;
    }

    float EaseOutBounce(float t) {
        const float n1 = 7.5625f;
        const float d1 = 2.75f;

        if (t < 1.0f / d1) {
            return n1 * t * t;
        } else if (t < 2.0f / d1) {
            t -= 1.5f / d1;
            return n1 * t * t + 0.75f;
        } else if (t < 2.5f / d1) {
            t -= 2.25f / d1;
            return n1 * t * t + 0.9375f;
        } else {
            t -= 2.625f / d1;
            return n1 * t * t + 0.984375f;
        }
    }

    float EaseInBounce(float t) {
        return 1.0f - EaseOutBounce(1.0f - t);
    }

    float EaseInOutBounce(float t) {
        return t < 0.5f ? (1.0f - EaseOutBounce(1.0f - 2.0f * t)) / 2.0f
                        : (1.0f + EaseOutBounce(2.0f * t - 1.0f)) / 2.0f;
    }
}