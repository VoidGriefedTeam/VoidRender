#pragma once
// ============================================================================
//  .mapb binary format  (version 2)  -- little endian, no padding
//  Version 1 files are still readable (they have none of the [v2] fields).
//
//  FILE
//    char[4]  magic = "VMAP"
//    u32      version            (1 or 2)
//    u32      object_count
//    str      map_name
//    CAMERA
//    LIGHTING                    [v2 only]
//    OBJECT * object_count
//
//  str     = u32 length, then `length` bytes (no terminator)
//  angles  = degrees
//
//  CAMERA
//    f32 x, y, z, fov
//    f32 yaw, pitch, roll        [v2]   (yaw is Camera::dir)
//    str name
//    u32 rotation_count
//    f32 rotation[rotation_count]
//
//  LIGHTING                      [v2]
//    f32 ambient[3]
//    u32 light_count
//    LIGHT * light_count
//
//  LIGHT   (88 bytes)
//    u32 type                    0 = point, 1 = directional
//    u32 flags                   bit0 = follow_camera
//    f32 position[3]             point; with follow_camera: offset in camera space
//    f32 direction[3]            directional: the way the light travels
//    f32 color[3]
//    f32 intensity
//    f32 range                   0 = no falloff
//    f32 orbit_center[3]
//    f32 orbit_radius            > 0 enables the orbit
//    f32 orbit_height
//    f32 orbit_speed             degrees per second
//    f32 orbit_phase             degrees
//
//  OBJECT
//    str name
//    u32 vertex_count, rotation_count, face_count, color_count
//    f32 yaw, pitch, roll        [v2]
//    u32 flags                   [v2]   bit0 = custom_pivot
//    f32 pivot[3]                [v2]   used when custom_pivot, else centroid
//    f32 x[vertex_count]
//    f32 y[vertex_count]
//    f32 z[vertex_count]
//    f32 rotation[rotation_count]
//    u32 face_len[face_count];   i32 face_data[sum(face_len)]
//    u32 color_len[color_count]; f32 color_data[sum(color_len)]
//
//  Every array is a contiguous block, so the reader loads it with one memcpy.
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
    inline constexpr std::uint32_t       version     = 2;   // written by mapc
    inline constexpr std::uint32_t       min_version = 1;   // oldest the reader accepts

    inline constexpr std::size_t light_bytes = 88;          // see LIGHT above
}
