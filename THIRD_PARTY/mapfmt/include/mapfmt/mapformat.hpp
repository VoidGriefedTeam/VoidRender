#pragma once
// ============================================================================
//  .mapb binary format  (version 2, "MAP2")  -- little endian, no padding
//  Version 1 files still load (no lights, ambient 0.15, all angles 0).
//
//  FILE
//    char[4]  magic = "VMAP"
//    u32      version (1 or 2)
//    u32      object_count
//    str      map_name
//    CAMERA
//    LIGHTING                       (v2 only)
//    OBJECT * object_count
//
//  str     = u32 length, then `length` bytes (no terminator)
//
//  CAMERA
//    f32 x, y, z, fov
//    f32 yaw, pitch, roll           (v2 only, degrees)
//    str name
//    u32 rotation_count
//    f32 rotation[rotation_count]
//
//  LIGHTING                         (v2 only)
//    f32 ambient[3]
//    u32 light_count
//    LIGHT * light_count
//
//  LIGHT  (80 bytes)
//    u32 type (0 point, 1 directional)
//    u32 flags (bit0 follow_camera)
//    f32 position[3], direction[3], color[3]
//    f32 intensity, range
//    f32 orbit_center[3]
//    f32 orbit_radius, orbit_height, orbit_speed, orbit_phase
//
//  OBJECT
//    str name
//    u32 vertex_count, rotation_count, face_count, color_count
//    f32 yaw, pitch, roll           (v2 only, degrees)
//    u32 flags (bit0 custom_pivot)  (v2 only)
//    f32 pivot[3]                   (v2 only)
//    f32 x[vertex_count]
//    f32 y[vertex_count]
//    f32 z[vertex_count]
//    f32 rotation[rotation_count]
//    u32 face_len[face_count];   i32 face_data[sum(face_len)]
//    u32 color_len[color_count]; f32 color_data[sum(color_len)]
//
//  Every array is a contiguous block, so the reader loads it with one memcpy.
//  Not stored in the file (runtime only): Object::visible, Object::emissive.
// ============================================================================
#include <array>
#include <bit>
#include <limits>
#include <cstdint>

static_assert(std::endian::native == std::endian::little,
              "mapb is little-endian; add byte-swapping for big-endian targets");
static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559,
              "mapb requires IEEE-754 32-bit floats");

namespace mapfmt
{
    inline constexpr std::array<char, 4> magic{'V', 'M', 'A', 'P'};
    inline constexpr std::uint32_t       version = 2;     // what mapc writes
    inline constexpr std::uint32_t       min_version = 1; // oldest the reader accepts

    inline constexpr std::uint32_t light_flag_follow_camera = 1u << 0;
    inline constexpr std::uint32_t object_flag_custom_pivot = 1u << 0;

    // On-disk light record. Trivially copyable, so the reader loads all lights with one memcpy.
    struct LightRecord
    {
        std::uint32_t type;   // 0 point, 1 directional
        std::uint32_t flags;
        float position[3];
        float direction[3];
        float color[3];
        float intensity, range;
        float orbit_center[3];
        float orbit_radius, orbit_height, orbit_speed, orbit_phase;
    };
    static_assert(sizeof(LightRecord) == 80, "LIGHT record must be 80 bytes");
}
