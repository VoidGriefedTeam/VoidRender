#pragma once
#include <SimpleGL.h>
#include <VoidCLcompute.h>
#include <render_export.h>
#include <map_obj.hpp>
#include <mapfmt/MapReader.hpp>

#include <math.h>
#include <array>
#include <functional>
#include <numbers>
#include <cstdio>
#include <vector>
#include <print>
#include <string>
#include <iostream>

#include <glaze/glaze.hpp>


// --------------------------------------------
// Glaze  (global renderer settings; lights themselves live in the map)
// --------------------------------------------

struct WindowConfig
{
    int width = 640;
    int height = 360;
    std::string title = "VoidRender";
    bool resizable = true;
    std::vector<float> bgcolor = {0.0f, 0.0f, 0.0f};
    int render_dist = 10;
};

// Flip-book animation: when enabled, every Object in the map is one frame
// and only one is drawn at a time. All keys are optional in config.json:
//   "animation": { "enabled": true, "fps": 12 }
struct AnimConfig
{
    bool enabled = false;
    float fps = 12.0f;
};

// Global lighting switches. The lights and the ambient colour come from the map.
//   "lighting": { "enabled": true, "specular": 0.3, "shininess": 32, "two_sided": true }
// two_sided flips normals toward the viewer (right for open surfaces like smoke).
struct LightingConfig
{
    bool enabled = true;
    float specular = 0.0f;
    float shininess = 32.0f;
    bool two_sided = true;
};

// Software occlusion culling. The draw list is rebuilt whenever the camera
// moves, so this is paid per camera change.
//   resolution_scale: occlusion buffer size relative to the window (0.1 - 1).
//                     Lower = faster, may drop very small faces.
//   depth_epsilon:    relative depth tolerance; larger keeps more faces.
struct OcclusionConfig
{
    bool enabled = true;
    float resolution_scale = 0.5f;
    float depth_epsilon = 0.005f;
};

struct Config
{
    WindowConfig window;
    AnimConfig animation;
    LightingConfig lighting;
    OcclusionConfig occlusion;
};




class RENDER_EXPORT Render
{
public:
    // Called once per frame BEFORE drawing. Poll your input however you like and
    // change map.camera (x, y, z, dir=yaw, pitch, roll, fov) and/or any
    // map.objects[i].yaw / pitch / roll / pivot; the next frame uses them.
    //   dt   = seconds since the previous frame
    //   time = seconds since rendering started
    using FrameCallback = std::function<void(Map& map, double dt, double time)>;

    Map load_map(const std::string& map_file);
    void load_config(const std::string& config_file);
    void render_map(Map& mapr);
    void update_cam(const int& x, const int& y, const int& z, Camera& cameraa,const float dir,const float& fov);
    void set_frame_callback(FrameCallback cb) { frameCallback = std::move(cb); }

private:
    float rad(float degrees);
    struct FOVcull
    {
        float Xa,Xb,Xc;
        float Za,Zb,Zc;
    };
    Config config;
    FOVcull cu;
    FrameCallback frameCallback;

    bool fov_cull(const float& Xp, const float& Zp);
};
inline Render render;
