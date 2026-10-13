#pragma once
// MapReader.hpp - loads .mapb files produced by mapc into your Map struct.
//
//   auto map = mapfmt::load_map("level.mapb");        // std::expected<Map, Error>
//   if (!map) { /* mapfmt::to_string(map.error()) */ }
//
//   auto file = mapfmt::load_file("level.mapb");      // also gives map/object names
//   file->name, file->object_names[i], file->map
//
// Requires C++23 (std::expected).
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <filesystem>
#include <fstream>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <mapfmt/Render.hpp> // Object, Camera, Map
#include <mapfmt/mapformat.hpp>

namespace mapfmt
{
enum class Error { FileOpen, FileRead, BadMagic, BadVersion, Truncated, BadData };

constexpr std::string_view to_string(Error e) noexcept
{
    switch (e)
    {
        case Error::FileOpen:   return "cannot open file";
        case Error::FileRead:   return "cannot read file";
        case Error::BadMagic:   return "not a mapb file";
        case Error::BadVersion: return "unsupported mapb version";
        case Error::Truncated:  return "file is truncated or corrupt";
        case Error::BadData:    return "file contains invalid data";
    }
    return "unknown error";
}

struct MapFile
{
    std::string              name;
    std::vector<std::string> object_names; // same order as map.objects
    std::string              camera_name;
    Map                      map;
};

namespace detail
{
// Assign 3 floats to whatever your vec3 is: std::array / C array / anything with operator[],
// or a struct with x, y, z members.
template <class V>
void set3(V& dst, const float (&src)[3]) noexcept
{
    if constexpr (requires { dst[0]; })
    {
        dst[0] = src[0]; dst[1] = src[1]; dst[2] = src[2];
    }
    else
    {
        dst.x = src[0]; dst.y = src[1]; dst.z = src[2];
    }
}

class Cursor
{
public:
    explicit Cursor(std::span<const std::byte> d) noexcept : data_(d) {}

    [[nodiscard]] size_t remaining() const noexcept { return data_.size() - pos_; }

    template <class T>
    [[nodiscard]] bool read(T& out) noexcept
    {
        if (remaining() < sizeof(T)) return false;
        std::memcpy(&out, data_.data() + pos_, sizeof(T));
        pos_ += sizeof(T);
        return true;
    }

    [[nodiscard]] bool read_string(std::string& s)
    {
        std::uint32_t n;
        if (!read(n) || n > remaining()) return false;
        s.assign(reinterpret_cast<const char*>(data_.data() + pos_), n);
        pos_ += n;
        return true;
    }

    // One bounds check + one memcpy. The size check happens BEFORE allocating,
    // so a corrupt count can never trigger a giant allocation.
    template <class T>
    [[nodiscard]] bool read_array(std::vector<T>& out, std::uint32_t n)
    {
        if (n > remaining() / sizeof(T)) return false;
        out.resize(n);
        if (n) std::memcpy(out.data(), data_.data() + pos_, size_t{n} * sizeof(T));
        pos_ += size_t{n} * sizeof(T);
        return true;
    }

    template <class T>
    [[nodiscard]] bool read_rows(std::vector<std::vector<T>>& out, std::uint32_t rows)
    {
        if (rows > remaining() / sizeof(std::uint32_t)) return false;
        const size_t lens_at = pos_;
        pos_ += size_t{rows} * sizeof(std::uint32_t);

        out.resize(rows);
        for (std::uint32_t i = 0; i < rows; ++i)
        {
            std::uint32_t len;
            std::memcpy(&len, data_.data() + lens_at + size_t{i} * sizeof(std::uint32_t), sizeof len);
            if (!read_array(out[i], len)) return false;
        }
        return true;
    }

private:
    std::span<const std::byte> data_;
    size_t                     pos_ = 0;
};
} // namespace detail

// Parse from memory (e.g. an embedded resource or your own mmap).
[[nodiscard]] inline std::expected<MapFile, Error> parse(std::span<const std::byte> bytes)
{
    using detail::Cursor;
    using std::unexpected;

    Cursor c(bytes);

    std::array<char, 4> mg{};
    std::uint32_t       ver = 0, object_count = 0;
    if (!c.read(mg) || mg != magic) return unexpected(Error::BadMagic);
    if (!c.read(ver) || ver < min_version || ver > version) return unexpected(Error::BadVersion);
    const bool v2 = ver >= 2;
    if (!c.read(object_count)) return unexpected(Error::Truncated);

    MapFile out;
    if (!c.read_string(out.name)) return unexpected(Error::Truncated);

    // camera
    Camera& cam = out.map.camera;
    std::uint32_t rot_n = 0;
    if (!c.read(cam.x) || !c.read(cam.y) || !c.read(cam.z) || !c.read(cam.fov))
        return unexpected(Error::Truncated);
    if (v2 && (!c.read(cam.dir) || !c.read(cam.pitch) || !c.read(cam.roll)))
        return unexpected(Error::Truncated);
    if (!c.read_string(out.camera_name) || !c.read(rot_n) || !c.read_array(cam.rotation, rot_n))
        return unexpected(Error::Truncated);

    // lighting (v1 files keep the Map defaults: ambient 0.15, no lights)
    if (v2)
    {
        float         ambient[3];
        std::uint32_t light_count = 0;
        if (!c.read(ambient) || !c.read(light_count)) return unexpected(Error::Truncated);
        detail::set3(out.map.ambient, ambient);

        std::vector<LightRecord> recs;
        if (!c.read_array(recs, light_count)) return unexpected(Error::Truncated);
        out.map.lights.resize(light_count);
        for (std::uint32_t i = 0; i < light_count; ++i)
        {
            const LightRecord& r = recs[i];
            Light&             l = out.map.lights[i];
            if (r.type > 1) return unexpected(Error::BadData);
            l.type          = static_cast<decltype(l.type)>(r.type);
            l.follow_camera = (r.flags & light_flag_follow_camera) != 0;
            detail::set3(l.position, r.position);
            detail::set3(l.direction, r.direction);
            detail::set3(l.color, r.color);
            l.intensity     = r.intensity;
            l.range         = r.range;
            detail::set3(l.orbit.center, r.orbit_center);
            l.orbit.radius  = r.orbit_radius;
            l.orbit.height  = r.orbit_height;
            l.orbit.speed   = r.orbit_speed;
            l.orbit.phase   = r.orbit_phase;
        }
    }

    // each object needs at least 5 u32 (name len + 4 counts), v2 adds 3 f32 + u32 + 3 f32
    if (object_count > c.remaining() / (v2 ? 48 : 20)) return unexpected(Error::Truncated);
    out.map.objects.resize(object_count);
    out.object_names.resize(object_count);

    for (std::uint32_t i = 0; i < object_count; ++i)
    {
        Object& o = out.map.objects[i];
        std::uint32_t vc, rc, fc, cc;

        if (!c.read_string(out.object_names[i]) ||
            !c.read(vc) || !c.read(rc) || !c.read(fc) || !c.read(cc))
            return unexpected(Error::Truncated);

        if (v2)
        {
            std::uint32_t flags = 0;
            float         pivot[3];
            if (!c.read(o.yaw) || !c.read(o.pitch) || !c.read(o.roll) ||
                !c.read(flags) || !c.read(pivot))
                return unexpected(Error::Truncated);
            o.custom_pivot = (flags & object_flag_custom_pivot) != 0;
            detail::set3(o.pivot, pivot);
        }

        if (!c.read_array(o.x, vc) || !c.read_array(o.y, vc) || !c.read_array(o.z, vc) ||
            !c.read_array(o.rotation, rc) ||
            !c.read_rows(o.faces, fc) ||
            !c.read_rows(o.color, cc))
            return unexpected(Error::Truncated);
    }
    return out;
}

// Load a whole file with a single read().
[[nodiscard]] inline std::expected<MapFile, Error> load_file(const std::filesystem::path& path)
{
    std::ifstream in(path, std::ios::binary | std::ios::ate);
    if (!in) return std::unexpected(Error::FileOpen);

    const auto size = in.tellg();
    if (size < 0) return std::unexpected(Error::FileRead);
    in.seekg(0);

    std::vector<std::byte> buf(static_cast<size_t>(size));
    if (!in.read(reinterpret_cast<char*>(buf.data()), size)) return std::unexpected(Error::FileRead);

    return parse(buf);
}

// Convenience: just the Map.
[[nodiscard]] inline std::expected<Map, Error> load_map(const std::filesystem::path& path)
{
    auto f = load_file(path);
    if (!f) return std::unexpected(f.error());
    return std::move(f->map);
}
} // namespace mapfmt
