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
        std::vector<float> rotation;            // legacy, not used by the renderer
        std::vector<std::vector<int>> faces;

        // Runtime orientation (can be changed every frame from the frame callback).
        // Rotates about the pivot: the centroid of the vertices unless custom_pivot is set.
        float yaw = 0.0f, pitch = 0.0f, roll = 0.0f;
        bool custom_pivot = false;
        std::array<float, 3> pivot = {0.0f, 0.0f, 0.0f};
};

struct Camera
{
    public:
        float x = 0.0f, y = 0.0f, z = 0.0f;
        float fov = 90.0f;
        std::vector<float> rotation;            // legacy, not used by the renderer
        float dir = 0.0f;                       // yaw in degrees (0 looks down -Z)

        float pitch = 0.0f;                     // positive = look up
        float roll  = 0.0f;                     // positive = head tilts left
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

struct Light
{
    public:
        enum class Type : unsigned char { Point = 0, Directional = 1 };

        Type type = Type::Point;
        std::array<float, 3> position  = {0.0f, 10.0f, 0.0f};   // Point (see follow_camera)
        std::array<float, 3> direction = {0.0f, -1.0f, 0.0f};   // Directional: way the light travels
        std::array<float, 3> color     = {1.0f, 1.0f, 1.0f};
        float intensity = 1.0f;
        float range = 0.0f;              // Point: fades to 0 at this distance (0 = no falloff)
        bool  follow_camera = false;     // Point: position is an offset in CAMERA space
                                         //        (x right, y up, -z forward), so {0,0,0} = headlight
        LightOrbit orbit;
};

struct Map
{
    public:
        std::vector<Object> objects;
        Camera camera;
        std::vector<Light> lights;
        std::array<float, 3> ambient = {0.15f, 0.15f, 0.15f};
};
