#pragma once
#include "raylib.h"
#include "raymath.h"
#include <vector>
#include <algorithm>
#include <limits>

class Polygon {
public:
    explicit Polygon(const std::vector<Vector2>& points,
                      Color fillColor = LIGHTGRAY,
                      Color outlineColor = BLACK)
        : vertices(points), fill(fillColor), outline(outlineColor)
    {
        RebuildFan();
    }


    void Draw() const {
        if (fanPoints.size() >= 3) {
            DrawTriangleFan(const_cast<Vector2*>(fanPoints.data()),
                             (int)fanPoints.size(), fill);
        }
    }

    void DrawOutline(float thickness = 1.0f) const {
        if (vertices.size() < 2) return;
        for (size_t i = 0; i < vertices.size(); i++) {
            Vector2 a = vertices[i];
            Vector2 b = vertices[(i + 1) % vertices.size()];
            DrawLineEx(a, b, thickness, outline);
        }
    }

    void DrawFilledWithOutline(float thickness = 1.0f) const {
        Draw();
        DrawOutline(thickness);
    }

    void SetPosition(Vector2 newCentroid) {
        Vector2 delta = Vector2Subtract(newCentroid, GetCentroid());
        Move(delta);
    }

    void Move(Vector2 delta) {
        for (auto& v : vertices) v = Vector2Add(v, delta);
        RebuildFan();
    }

    void Rotate(float degrees, Vector2 pivot) {
        float rad = degrees * DEG2RAD;
        float s = sinf(rad), c = cosf(rad);
        for (auto& v : vertices) {
            Vector2 p = Vector2Subtract(v, pivot);
            Vector2 rotated = { p.x * c - p.y * s, p.x * s + p.y * c };
            v = Vector2Add(rotated, pivot);
        }
        RebuildFan();
    }

    void Scale(float factor, Vector2 pivot) {
        for (auto& v : vertices) {
            Vector2 p = Vector2Subtract(v, pivot);
            v = Vector2Add(Vector2Scale(p, factor), pivot);
        }
        RebuildFan();
    }


    Vector2 GetCentroid() const {
        if (vertices.empty()) return { 0, 0 };
        Vector2 sum = { 0, 0 };
        for (auto& v : vertices) sum = Vector2Add(sum, v);
        return Vector2Scale(sum, 1.0f / vertices.size());
    }

    Rectangle GetBoundingBox() const {
        if (vertices.empty()) return { 0, 0, 0, 0 };
        float minX = std::numeric_limits<float>::max();
        float minY = std::numeric_limits<float>::max();
        float maxX = std::numeric_limits<float>::lowest();
        float maxY = std::numeric_limits<float>::lowest();
        for (auto& v : vertices) {
            minX = std::min(minX, v.x);
            minY = std::min(minY, v.y);
            maxX = std::max(maxX, v.x);
            maxY = std::max(maxY, v.y);
        }
        return { minX, minY, maxX - minX, maxY - minY };
    }

    const std::vector<Vector2>& GetVertices() const { return vertices; }

    bool CheckCollisionPoint(Vector2 point) const {
        Rectangle box = GetBoundingBox();
        if (!CheckCollisionPointRec(point, box)) return false;

        bool inside = false;
        size_t n = vertices.size();
        for (size_t i = 0, j = n - 1; i < n; j = i++) {
            const Vector2& vi = vertices[i];
            const Vector2& vj = vertices[j];
            bool intersect = ((vi.y > point.y) != (vj.y > point.y)) &&
                (point.x < (vj.x - vi.x) * (point.y - vi.y) / (vj.y - vi.y) + vi.x);
            if (intersect) inside = !inside;
        }
        return inside;
    }

    bool CheckCollisionCircle(Vector2 center, float radius) const {
        if (CheckCollisionPoint(center)) return true;
        size_t n = vertices.size();
        for (size_t i = 0; i < n; i++) {
            Vector2 a = vertices[i];
            Vector2 b = vertices[(i + 1) % n];
            if (CheckCollisionCircleLine(center, radius, a, b)) return true;
        }
        return false;
    }

    bool CheckCollisionBoundingBox(const Polygon& other) const {
        return CheckCollisionRecs(GetBoundingBox(), other.GetBoundingBox());
    }

    bool IsHovered() const {
        return CheckCollisionPoint(GetMousePosition());
    }

    bool IsClicked(int button = MOUSE_BUTTON_LEFT) const {
            return IsHovered() && IsMouseButtonPressed(button);
        }

    bool IsPressed(int button = MOUSE_BUTTON_LEFT) const {
        return IsHovered() && IsMouseButtonDown(button);
    }

    bool IsReleased(int button = MOUSE_BUTTON_LEFT) const {
        return IsHovered() && IsMouseButtonReleased(button);
    }

    void SetFillColor(Color c) { fill = c; }
    void SetOutlineColor(Color c) { outline = c; }
    Color GetFillColor() const { return fill; }
    Color GetOutlineColor() const { return outline; }

    void SetVertices(const std::vector<Vector2>& points) {
        vertices = points;
        RebuildFan();
    }

private:
    std::vector<Vector2> vertices;
    std::vector<Vector2> fanPoints;
    Color fill;
    Color outline;

    static bool CheckCollisionCircleLine(Vector2 center, float radius, Vector2 a, Vector2 b) {
        Vector2 ab = Vector2Subtract(b, a);
        Vector2 ac = Vector2Subtract(center, a);
        float lenSq = ab.x * ab.x + ab.y * ab.y;
        float t = lenSq > 0.0f ? Clamp(Vector2DotProduct(ac, ab) / lenSq, 0.0f, 1.0f) : 0.0f;
        Vector2 closest = Vector2Add(a, Vector2Scale(ab, t));
        float distSq = Vector2DistanceSqr(center, closest);
        return distSq <= radius * radius;
    }

    void RebuildFan() {
        fanPoints.clear();
        if (vertices.size() < 3) return;
        fanPoints.reserve(vertices.size() + 2);
        fanPoints.push_back(GetCentroid());
        for (auto& v : vertices) fanPoints.push_back(v);
        fanPoints.push_back(vertices[0]);
    }
};