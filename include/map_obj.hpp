#pragma once
#include <array>
#include <vector>

// --------------------------------------------
// Render
// --------------------------------------------

// Angles are in DEGREES. Orientation = Ry(yaw) * Rx(pitch) * Rz(roll):
//   yaw   : turn around the vertical (Y) axis
//   pitch : tilt around X   (positive = nose up)
//   roll  : spin around Z   (the object's / camera's forward axis)
struct Object
{
    public:
        std::vector<float> x,y,z;
        std::vector<std::vector<float>> color;
        std::vector<float> rotation;
        std::vector<std::vector<int>> faces;
        float yaw = 0, pitch = 0, roll = 0;
        bool  custom_pivot = false;
        std::array<float, 3> pivot{0, 0, 0};
        // runtime only, not stored in .mapb
        bool  visible  = true;
        float emissive = 0;
    private:
        std::vector<float> tmpx, tmpy, tmpz;
        friend class Render;
};

struct Camera
{
    public:
        float x,y,z;
        float fov;
        float dir = 0, pitch = 0, roll = 0; // dir = yaw
        std::vector<float> rotation;
};

// Circular light motion in the XZ plane (radius > 0 enables it):
//   pos = center + (cos(a)*radius, height, sin(a)*radius),  a = speed*t + phase  (degrees, t in s)
struct LightOrbit
{
    public:
        std::array<float, 3> center = {0.0f, 0.0f, 0.0f};
        float radius = 0.0f;
        float height = 0.0f;
        float speed  = 0.0f;
        float phase  = 0.0f;
};

struct Orbit
{
    std::array<float, 3> center{0, 0, 0};
    float radius = 0; // circles in the XZ plane when > 0
    float height = 0; // added to center.y
    float speed  = 0; // deg/s, negative = other way
    float phase  = 0; // deg
};

struct Light
{
    enum class Type { Point = 0, Directional = 1 };

    Type                 type = Type::Point;
    std::array<float, 3> position{0, 0, 0};   // Point (offset in camera space when follow_camera)
    std::array<float, 3> direction{0, -1, 0}; // Directional: the way the light TRAVELS
    std::array<float, 3> color{1, 1, 1};
    float                intensity = 1;
    float                range     = 0; // Point: 0 = no falloff
    bool                 follow_camera = false;
    Orbit                orbit;
};


struct Map
{
    public:
        std::vector<Object> objects;
        Camera camera;
        std::array<float, 3> ambient{0.15f, 0.15f, 0.15f};
        std::vector<Light>   lights; // none = lighting off
};


