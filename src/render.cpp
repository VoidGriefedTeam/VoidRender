#include <render.hpp>
#include <algorithm>
#include <chrono>
#include <cmath>

float Render::rad(float degrees)
{
    return degrees * std::numbers::pi_v<float> / 180.0f;
}

void Render::load_config(const std::string& config_file)
{
    std::string buffer;

    auto error = glz::read_file_json(config, config_file, buffer);

    if (error)
    {
        std::println(
            "Failed to load config: {}",
            glz::format_error(error)
        );
        return;
    }

    std::println(
        "Window: {}x{}",
        config.window.width,
        config.window.height
    );
}

Map Render::load_map(const std::string& map_file)
{

    auto result = mapfmt::load_map(map_file);
    if (!result)
    {
        std::cerr << mapfmt::to_string(result.error()) << '\n';
        std::exit(104);
    }

    Map map = std::move(*result);

    return map;
}

    //FOV culling
void Render::update_cam(const int& x, const int& y, const int& z, Camera& cameraa,const float dir,const float& fov)
{
    cameraa.x = x;
    cameraa.y = y;
    cameraa.z = z;
    cameraa.fov = fov;
    cameraa.dir = dir;

    auto Xd = -config.window.render_dist*sin(rad(cameraa.dir));
    auto Zd = -config.window.render_dist*cos(rad(cameraa.dir));
    float AD = 2*config.window.render_dist;
    float AC, AB = AC = config.window.render_dist/cos(rad(cameraa.fov)/2);
    float BC = 2*config.window.render_dist*tan(rad(cameraa.fov)/2);
    cu.Xb = Xd - ((BC*Zd)/AB);
    cu.Zb = Zd + ((BC*Xd)/AB);
    cu.Xc = Xd + ((BC*Zd)/AD);
    cu.Zc = Zd - ((BC*Xd)/AD);
    cu.Xa = cameraa.x;
    cu.Za = cameraa.z;
}

bool Render::fov_cull(const float& Xp, const float& Zp)
{
    auto d1 = ((Xp-cu.Xa)*(cu.Zb-cu.Za))-((Zp-cu.Za)*(cu.Xb-cu.Xa));
    auto d2 = ((Xp-cu.Xb)*(cu.Zc-cu.Zb))-((Zp-cu.Zb)*(cu.Xc-cu.Xb));
    auto d3 = ((Xp-cu.Xc)*(cu.Za-cu.Zc))-((Zp-cu.Zc)*(cu.Xa-cu.Xc));

    bool view = (d1 >= 0 && d2 >= 0 && d3 >=0)
                || (d1 <= 0 && d2 <= 0 && d3 <= 0);
    return view;
}

using std::vector;

// Back-face culling convention. Faces are expected to be wound
// counter-clockwise when seen from OUTSIDE (normal = (v1-v0) x (v2-v0)
// points outward), which is how the smoke maps are generated.
// If you only see the inside of the smoke, set this to true.
static constexpr bool FLIP_BACKFACE_TEST = false;

namespace
{
    // Scan-converts one triangle into a W x H buffer.
    //   i0..i2  : per-vertex value interpolated across the triangle (1/depth)
    //   margin  : dilates the triangle by this many pixels (0 = exact
    //             pixel-centre sampling, > 0 = conservative coverage)
    //   fn(x,y,value) is called per covered pixel; returning true stops early
    //   and makes raster_triangle return true.
    template <class Fn>
    bool raster_triangle(float x0, float y0, float i0,
                         float x1, float y1, float i1,
                         float x2, float y2, float i2,
                         int W, int H, float margin, Fn&& fn)
    {
        const float area = (x1 - x0) * (y2 - y0) - (x2 - x0) * (y1 - y0);
        if (std::fabs(area) < 1e-6f)
            return false;

        const float absArea = std::fabs(area);
        const float sgn = area > 0.0f ? 1.0f : -1.0f;

        const float t0 = -margin * std::hypot(x2 - x1, y2 - y1);
        const float t1 = -margin * std::hypot(x0 - x2, y0 - y2);
        const float t2 = -margin * std::hypot(x1 - x0, y1 - y0);

        const int minX = std::max(0,     static_cast<int>(std::floor(std::min({x0, x1, x2}) - margin)));
        const int maxX = std::min(W - 1, static_cast<int>(std::ceil (std::max({x0, x1, x2}) + margin)));
        const int minY = std::max(0,     static_cast<int>(std::floor(std::min({y0, y1, y2}) - margin)));
        const int maxY = std::min(H - 1, static_cast<int>(std::ceil (std::max({y0, y1, y2}) + margin)));

        const float lo = std::min({i0, i1, i2});
        const float hi = std::max({i0, i1, i2});

        for (int y = minY; y <= maxY; ++y)
        {
            const float py = static_cast<float>(y) + 0.5f;
            for (int x = minX; x <= maxX; ++x)
            {
                const float px = static_cast<float>(x) + 0.5f;

                const float w0 = ((x2 - x1) * (py - y1) - (y2 - y1) * (px - x1)) * sgn;
                const float w1 = ((x0 - x2) * (py - y2) - (y0 - y2) * (px - x2)) * sgn;
                const float w2 = ((x1 - x0) * (py - y0) - (y1 - y0) * (px - x0)) * sgn;

                if (w0 < t0 || w1 < t1 || w2 < t2)
                    continue;

                // 1/depth is linear in screen space.  With dilation the sample
                // can fall slightly outside, so clamp to the vertex range.
                float v = (w0 * i0 + w1 * i1 + w2 * i2) / absArea;
                v = std::clamp(v, lo, hi);

                if (fn(x, y, v))
                    return true;
            }
        }
        return false;
    }

    struct ActiveLight
    {
        bool directional;
        float x, y, z;      // position, or the unit vector TOWARD the light
        float r, g, b;      // colour * intensity
        float range;
    };

    struct CamState
    {
        float x, y, z, dir, pitch, roll, fov;
        bool operator==(const CamState&) const = default;
    };

    struct ObjState
    {
        float yaw, pitch, roll;
        bool custom;
        float pivX, pivY, pivZ;
        bool operator==(const ObjState&) const = default;
    };

    inline ObjState make_state(const Object& o)
    {
        return {o.yaw, o.pitch, o.roll, o.custom_pivot, o.pivot[0], o.pivot[1], o.pivot[2]};
    }

    // 3x3 row-major matrix.
    struct Mat3 { float m[9]; };

    inline Mat3 mat_mul(const Mat3& a, const Mat3& b)
    {
        Mat3 r{};
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
            {
                float s = 0.0f;
                for (int k = 0; k < 3; ++k)
                    s += a.m[i * 3 + k] * b.m[k * 3 + j];
                r.m[i * 3 + j] = s;
            }
        return r;
    }

    inline Mat3 mat_transpose(const Mat3& a)
    {
        Mat3 r{};
        for (int i = 0; i < 3; ++i)
            for (int j = 0; j < 3; ++j)
                r.m[i * 3 + j] = a.m[j * 3 + i];
        return r;
    }

    inline void mat_apply(const Mat3& a, float x, float y, float z,
                          float& ox, float& oy, float& oz)
    {
        ox = a.m[0] * x + a.m[1] * y + a.m[2] * z;
        oy = a.m[3] * x + a.m[4] * y + a.m[5] * z;
        oz = a.m[6] * x + a.m[7] * y + a.m[8] * z;
    }

    // R = Ry(yaw) * Rx(pitch) * Rz(roll), angles in radians.
    // Yaw only: R * (0,0,-1) = (-sin yaw, 0, -cos yaw), same as the old camera.
    inline Mat3 euler_matrix(float yaw, float pitch, float roll)
    {
        const float cy = std::cos(yaw),   sy = std::sin(yaw);
        const float cp = std::cos(pitch), sp = std::sin(pitch);
        const float cr = std::cos(roll),  sr = std::sin(roll);

        const Mat3 ry{{ cy, 0, sy,   0, 1, 0,   -sy, 0, cy }};
        const Mat3 rx{{ 1, 0, 0,   0, cp, -sp,   0, sp, cp }};
        const Mat3 rz{{ cr, -sr, 0,   sr, cr, 0,   0, 0, 1 }};

        return mat_mul(ry, mat_mul(rx, rz));
    }
}

void Render::render_map(Map& mapr)
{

    GC_Init();
    sgl_init(
        config.window.width,
        config.window.height,
        config.window.title.c_str()
    );

    sgl_set_center_mode();

    int ofset = 0;

    decltype(mapr.objects[0].x) globalX,globalY,globalZ;

    vector<int> offset;
    offset.resize(mapr.objects.size());

    for (size_t i = 0; i < mapr.objects.size(); i++)
    {
        offset[i] = ofset;
        globalX.insert(globalX.end(), mapr.objects[i].x.begin(), mapr.objects[i].x.end());
        globalY.insert(globalY.end(), mapr.objects[i].y.begin(), mapr.objects[i].y.end());
        globalZ.insert(globalZ.end(), mapr.objects[i].z.begin(), mapr.objects[i].z.end());
        ofset += mapr.objects[i].x.size();
    }

    const size_t vertexCount = globalX.size();

    // Untransformed (object-space) positions. globalX/Y/Z become the WORLD
    // positions once each object's yaw/pitch/roll is applied (see apply_object).
    const auto localX = globalX;
    const auto localY = globalY;
    const auto localZ = globalZ;

    // ------------------------------------------------------------
    // Per-vertex colour (flattened) and smooth per-vertex normals.
    // Geometry is static, so both are computed once.  Colours are per
    // VERTEX, so lighting is evaluated per vertex and the rasteriser
    // interpolates the lit colours (Gouraud).
    // ------------------------------------------------------------
    size_t stride = 3;
    for (const auto& obj : mapr.objects)
        if (!obj.color.empty()) { stride = obj.color[0].size(); break; }

    vector<float> baseColor(vertexCount * stride, 1.0f);
    for (size_t o = 0; o < mapr.objects.size(); ++o)
    {
        const auto& obj = mapr.objects[o];
        for (size_t i = 0; i < obj.color.size(); ++i)
        {
            const size_t g = static_cast<size_t>(offset[o]) + i;
            if (g >= vertexCount) break;
            for (size_t k = 0; k < stride && k < obj.color[i].size(); ++k)
                baseColor[g * stride + k] = obj.color[i][k];
        }
    }

    const auto& L = config.lighting;
    const bool lighting = L.enabled && !mapr.lights.empty() && stride >= 3;
    if (L.enabled && !mapr.lights.empty() && stride < 3)
        std::println("Lighting disabled: vertex colours need at least 3 components.");

    vector<float> nrmX(vertexCount, 0.0f), nrmY(vertexCount, 0.0f), nrmZ(vertexCount, 0.0f);

    if (lighting)
    {
        // Newell's method per face (area weighted, works for any polygon),
        // accumulated on every vertex of the face, then normalised.
        for (size_t o = 0; o < mapr.objects.size(); ++o)
        {
            for (const auto& face : mapr.objects[o].faces)
            {
                const size_t n = face.size();
                if (n < 3) continue;

                float fx = 0.0f, fy = 0.0f, fz = 0.0f;
                for (size_t k = 0; k < n; ++k)
                {
                    const size_t a = static_cast<size_t>(offset[o] + face[k]);
                    const size_t b = static_cast<size_t>(offset[o] + face[(k + 1) % n]);
                    fx += (globalY[a] - globalY[b]) * (globalZ[a] + globalZ[b]);
                    fy += (globalZ[a] - globalZ[b]) * (globalX[a] + globalX[b]);
                    fz += (globalX[a] - globalX[b]) * (globalY[a] + globalY[b]);
                }
                for (size_t k = 0; k < n; ++k)
                {
                    const size_t a = static_cast<size_t>(offset[o] + face[k]);
                    nrmX[a] += fx; nrmY[a] += fy; nrmZ[a] += fz;
                }
            }
        }
        for (size_t v = 0; v < vertexCount; ++v)
        {
            const float len = std::sqrt(nrmX[v]*nrmX[v] + nrmY[v]*nrmY[v] + nrmZ[v]*nrmZ[v]);
            if (len > 1e-12f) { nrmX[v] /= len; nrmY[v] /= len; nrmZ[v] /= len; }
        }
    }

    // ------------------------------------------------------------
    // Object transforms.  Each object rotates about its pivot (centroid
    // unless custom_pivot).  Re-run for an object when its yaw/pitch/roll/
    // pivot changes: world position = pivot + R * (local - pivot),
    // world normal = R * local normal.
    // (At this point nrmX/Y/Z still hold the object-space normals.)
    // ------------------------------------------------------------
    const auto nrmLX = nrmX;
    const auto nrmLY = nrmY;
    const auto nrmLZ = nrmZ;

    vector<std::array<float, 3>> centroid(mapr.objects.size(), {0.0f, 0.0f, 0.0f});
    for (size_t o = 0; o < mapr.objects.size(); ++o)
    {
        const size_t n = mapr.objects[o].x.size();
        if (n == 0) continue;
        double sx = 0.0, sy = 0.0, sz = 0.0;
        for (size_t i = 0; i < n; ++i)
        {
            const size_t g = static_cast<size_t>(offset[o]) + i;
            sx += localX[g]; sy += localY[g]; sz += localZ[g];
        }
        centroid[o] = {static_cast<float>(sx / n), static_cast<float>(sy / n), static_cast<float>(sz / n)};
    }

    auto apply_object = [&](size_t o)
    {
        const Object& obj = mapr.objects[o];
        const Mat3 R = euler_matrix(rad(obj.yaw), rad(obj.pitch), rad(obj.roll));

        const float cx = obj.custom_pivot ? obj.pivot[0] : centroid[o][0];
        const float cy = obj.custom_pivot ? obj.pivot[1] : centroid[o][1];
        const float cz = obj.custom_pivot ? obj.pivot[2] : centroid[o][2];

        const size_t n = obj.x.size();
        for (size_t i = 0; i < n; ++i)
        {
            const size_t g = static_cast<size_t>(offset[o]) + i;

            float ox, oy, oz;
            mat_apply(R, localX[g] - cx, localY[g] - cy, localZ[g] - cz, ox, oy, oz);
            globalX[g] = cx + ox;
            globalY[g] = cy + oy;
            globalZ[g] = cz + oz;

            if (lighting)
                mat_apply(R, nrmLX[g], nrmLY[g], nrmLZ[g], nrmX[g], nrmY[g], nrmZ[g]);
        }
    };

    vector<ObjState> objState;
    objState.reserve(mapr.objects.size());
    for (size_t o = 0; o < mapr.objects.size(); ++o)
    {
        apply_object(o);
        objState.push_back(make_state(mapr.objects[o]));
    }

    // ------------------------------------------------------------
    // Camera projection.  Re-run whenever the camera or an object changes.
    //   rx/ry/rz : vertex position relative to the camera
    //   px/py    : projected, window-centre pixel units
    //   pd       : view depth (<= 0 means behind the camera)
    // ------------------------------------------------------------
    const float W  = static_cast<float>(config.window.width);
    const float H  = static_cast<float>(config.window.height);
    const float hw = W * 0.5f;
    const float hh = H * 0.5f;

    vector<float> rx(vertexCount), ry(vertexCount), rz(vertexCount);
    vector<float> px(vertexCount, 0.0f), py(vertexCount, 0.0f), pd(vertexCount, 0.0f);

    auto update_projection = [&]()
    {
        const Camera& cam = mapr.camera;
        const float focal = hw / std::tan(rad(cam.fov) / 2.0f);

        // world -> view = transpose of the camera orientation.
        // View space: +x right, +y up, -z forward (depth = -z).
        const Mat3 M = mat_transpose(euler_matrix(rad(cam.dir), rad(cam.pitch), rad(cam.roll)));

        for (size_t i = 0; i < vertexCount; ++i)
        {
            const float x = globalX[i] - cam.x;
            const float y = globalY[i] - cam.y;
            const float z = globalZ[i] - cam.z;
            rx[i] = x; ry[i] = y; rz[i] = z;

            float viewX, viewY, viewZ;
            mat_apply(M, x, y, z, viewX, viewY, viewZ);
            const float depth = -viewZ;

            if (depth <= 0.001f)
            {
                pd[i] = 0.0f;
                continue;
            }

            pd[i] = depth;
            px[i] = focal * viewX / depth;
            py[i] = focal * viewY / depth;
        }
    };

    // ------------------------------------------------------------
    // Draw lists (rebuilt only when the camera changes, then cached):
    //   - drop faces with any vertex behind the camera
    //   - drop back-facing faces
    //   - drop faces completely outside the window
    //   - occlusion culling against a software depth buffer
    //   - sort far -> near (painter's algorithm)
    // ------------------------------------------------------------
    struct DrawFace
    {
        float depth;
        vector<int>   idx;          // global vertex indices
        vector<float> x, y, c;      // screen coords, per-vertex colour (stride floats each)
    };

    struct FrameList
    {
        vector<DrawFace> faces;
        vector<int> used;           // unique global vertices referenced (for lighting)
        unsigned long long litVersion = 0;   // lights version these colours were computed for
    };

    const bool occlusion = config.occlusion.enabled;
    const float occScale = std::clamp(config.occlusion.resolution_scale, 0.1f, 1.0f);
    const int   OW = std::max(1, static_cast<int>(std::lround(W * occScale)));
    const int   OH = std::max(1, static_cast<int>(std::lround(H * occScale)));
    const float eps = std::max(0.0f, config.occlusion.depth_epsilon);

    vector<float> zbuf(static_cast<size_t>(OW) * OH);

    auto build_list = [&](size_t firstObj, size_t lastObj)
    {
        FrameList out;
        vector<DrawFace>& list = out.faces;

        for (size_t objIndex = firstObj; objIndex < lastObj; ++objIndex)
        {
            const auto& obj = mapr.objects[objIndex];

            for (const auto& face : obj.faces)
            {
                if (face.size() < 3)
                    continue;

                bool visible = true;
                float depthSum = 0.0f;
                for (const int localIndex : face)
                {
                    const size_t index = static_cast<size_t>(offset[objIndex] + localIndex);
                    if (pd[index] <= 0.001f) { visible = false; break; }
                    depthSum += pd[index];
                }
                if (!visible)
                    continue;

                // rx/ry/rz are positions relative to the camera, so the vector
                // from the camera to vertex 0 is simply (rx, ry, rz)[i0].
                const size_t i0 = static_cast<size_t>(offset[objIndex] + face[0]);
                const size_t i1 = static_cast<size_t>(offset[objIndex] + face[1]);
                const size_t i2 = static_cast<size_t>(offset[objIndex] + face[2]);

                const float e1x = rx[i1] - rx[i0], e1y = ry[i1] - ry[i0], e1z = rz[i1] - rz[i0];
                const float e2x = rx[i2] - rx[i0], e2y = ry[i2] - ry[i0], e2z = rz[i2] - rz[i0];

                const float nx = e1y * e2z - e1z * e2y;
                const float ny = e1z * e2x - e1x * e2z;
                const float nz = e1x * e2y - e1y * e2x;

                const float facing = nx * rx[i0] + ny * ry[i0] + nz * rz[i0];  // < 0: faces the camera
                const bool backFacing = FLIP_BACKFACE_TEST ? (facing <= 0.0f) : (facing >= 0.0f);
                if (backFacing)
                    continue;

                DrawFace df;
                df.depth = depthSum / static_cast<float>(face.size());
                df.idx.reserve(face.size());
                df.x.reserve(face.size());
                df.y.reserve(face.size());
                df.c.reserve(face.size() * stride);

                float minX = 1e30f, maxX = -1e30f, minY = 1e30f, maxY = -1e30f;

                for (const int localIndex : face)
                {
                    const size_t index = static_cast<size_t>(offset[objIndex] + localIndex);
                    df.idx.push_back(static_cast<int>(index));
                    df.x.push_back(px[index]);
                    df.y.push_back(py[index]);

                    minX = std::min(minX, px[index]); maxX = std::max(maxX, px[index]);
                    minY = std::min(minY, py[index]); maxY = std::max(maxY, py[index]);

                    // Unlit colour for now; relit when lighting is on.
                    df.c.insert(df.c.end(),
                                baseColor.begin() + index * stride,
                                baseColor.begin() + (index + 1) * stride);
                }

                // Completely outside the window -> never visible.
                if (maxX < -hw || minX > hw || maxY < -hh || minY > hh)
                    continue;

                list.push_back(std::move(df));
            }
        }

        // --------------------------------------------------------
        // Occlusion culling.
        //   Pass 1: rasterise every candidate (exact sampling) and keep
        //           the nearest 1/depth per pixel.
        //   Pass 2: a face survives if at least one pixel it covers
        //           (dilated, so it is conservative) is not hidden behind
        //           something clearly nearer.
        // --------------------------------------------------------
        if (occlusion && !list.empty())
        {
            std::fill(zbuf.begin(), zbuf.end(), 0.0f);   // 0 = empty

            auto bx = [&](float sx) { return (sx + hw) * occScale; };
            auto by = [&](float sy) { return (sy + hh) * occScale; };

            auto for_each_tri = [&](const DrawFace& df, auto&& body)
            {
                for (size_t k = 1; k + 1 < df.idx.size(); ++k)
                {
                    const size_t a = 0, b = k, c = k + 1;
                    if (body(bx(df.x[a]), by(df.y[a]), 1.0f / pd[df.idx[a]],
                             bx(df.x[b]), by(df.y[b]), 1.0f / pd[df.idx[b]],
                             bx(df.x[c]), by(df.y[c]), 1.0f / pd[df.idx[c]]))
                        return true;
                }
                return false;
            };

            for (const auto& df : list)
            {
                for_each_tri(df, [&](float x0, float y0, float i0,
                                     float x1, float y1, float i1,
                                     float x2, float y2, float i2)
                {
                    return raster_triangle(x0, y0, i0, x1, y1, i1, x2, y2, i2,
                                           OW, OH, 0.0f,
                                           [&](int x, int y, float inv)
                    {
                        float& z = zbuf[static_cast<size_t>(y) * OW + x];
                        if (inv > z) z = inv;
                        return false;
                    });
                });
            }

            vector<DrawFace> kept;
            kept.reserve(list.size());

            for (auto& df : list)
            {
                const bool seen = for_each_tri(df, [&](float x0, float y0, float i0,
                                                       float x1, float y1, float i1,
                                                       float x2, float y2, float i2)
                {
                    return raster_triangle(x0, y0, i0, x1, y1, i1, x2, y2, i2,
                                           OW, OH, 0.75f,
                                           [&](int x, int y, float inv)
                    {
                        return inv >= zbuf[static_cast<size_t>(y) * OW + x] * (1.0f - eps);
                    });
                });

                // Degenerate (zero-area on screen) faces cannot be tested; they
                // are invisible anyway, so they are dropped with the rest.
                if (seen)
                    kept.push_back(std::move(df));
            }

            list = std::move(kept);
        }

        std::sort(list.begin(), list.end(),
                  [](const DrawFace& a, const DrawFace& b) { return a.depth > b.depth; });

        vector<char> mark(vertexCount, 0);
        for (const auto& df : list)
            for (const int v : df.idx)
                if (!mark[v]) { mark[v] = 1; out.used.push_back(v); }

        return out;
    };

    const bool animate = config.animation.enabled && mapr.objects.size() > 1;
    const double fps = config.animation.fps > 0.0f ? config.animation.fps : 12.0;

    // One cache entry per animation frame (or a single entry for a static map).
    // All entries are invalidated when the camera moves.
    const size_t slots = animate ? mapr.objects.size() : 1;
    vector<FrameList> cache(slots);
    vector<char> valid(slots, 0);

    if (animate)
        std::println("Animation: {} frames @ {} fps", mapr.objects.size(), fps);

    // ------------------------------------------------------------
    // Lighting (per-vertex Blinn-Phong). Lights and ambient come from the map.
    // ------------------------------------------------------------
    float eyeX = 0.0f, eyeY = 0.0f, eyeZ = 0.0f;
    Mat3 camR{{ 1, 0, 0,   0, 1, 0,   0, 0, 1 }};   // camera orientation (view -> world)

    vector<ActiveLight> active;
    vector<float> sig, prevSig;                  // light state signature -> change detection
    unsigned long long lightsVersion = 1;
    vector<float> lit(lighting ? vertexCount * 3 : 0, 0.0f);

    auto update_lights = [&](double t)
    {
        active.clear();
        sig.clear();
        for (const float a : mapr.ambient) sig.push_back(a);

        for (const auto& l : mapr.lights)
        {
            ActiveLight a{};
            a.directional = (l.type == Light::Type::Directional);
            a.r = l.color[0] * l.intensity;
            a.g = l.color[1] * l.intensity;
            a.b = l.color[2] * l.intensity;
            a.range = l.range;

            if (a.directional)
            {
                // direction is where the light travels; we need the way back.
                const float dx = -l.direction[0], dy = -l.direction[1], dz = -l.direction[2];
                const float len = std::sqrt(dx*dx + dy*dy + dz*dz);
                if (len < 1e-8f) continue;
                a.x = dx / len; a.y = dy / len; a.z = dz / len;
            }
            else
            {
                float x = l.position[0], y = l.position[1], z = l.position[2];
                if (l.orbit.radius > 0.0f)
                {
                    const float ang = rad(l.orbit.speed * static_cast<float>(t) + l.orbit.phase);
                    x = l.orbit.center[0] + std::cos(ang) * l.orbit.radius;
                    y = l.orbit.center[1] + l.orbit.height;
                    z = l.orbit.center[2] + std::sin(ang) * l.orbit.radius;
                }
                if (l.follow_camera)
                {
                    // offset is in camera space, so it turns with yaw/pitch/roll
                    float ox, oy, oz;
                    mat_apply(camR, x, y, z, ox, oy, oz);
                    x = eyeX + ox; y = eyeY + oy; z = eyeZ + oz;
                }
                a.x = x; a.y = y; a.z = z;
            }

            active.push_back(a);
            for (const float v : {a.x, a.y, a.z, a.r, a.g, a.b, a.range})
                sig.push_back(v);
            sig.push_back(a.directional ? 1.0f : 0.0f);
        }

        if (sig != prevSig)
        {
            prevSig = sig;
            ++lightsVersion;
        }
    };

    auto light_vertex = [&](size_t v)
    {
        const float Px = globalX[v], Py = globalY[v], Pz = globalZ[v];
        float Nx = nrmX[v], Ny = nrmY[v], Nz = nrmZ[v];

        float Vx = eyeX - Px, Vy = eyeY - Py, Vz = eyeZ - Pz;
        const float vlen = std::sqrt(Vx*Vx + Vy*Vy + Vz*Vz);
        if (vlen > 1e-8f) { Vx /= vlen; Vy /= vlen; Vz /= vlen; }

        if (L.two_sided && (Nx*Vx + Ny*Vy + Nz*Vz) < 0.0f)
        {
            Nx = -Nx; Ny = -Ny; Nz = -Nz;
        }

        float dr = mapr.ambient[0], dg = mapr.ambient[1], db = mapr.ambient[2];
        float sr = 0.0f, sg = 0.0f, sb = 0.0f;

        for (const auto& li : active)
        {
            float lx, ly, lz, att = 1.0f;

            if (li.directional)
            {
                lx = li.x; ly = li.y; lz = li.z;
            }
            else
            {
                const float dx = li.x - Px, dy = li.y - Py, dz = li.z - Pz;
                const float d = std::sqrt(dx*dx + dy*dy + dz*dz);
                if (d < 1e-6f) continue;
                lx = dx / d; ly = dy / d; lz = dz / d;

                if (li.range > 0.0f)
                {
                    const float q = d / li.range;
                    const float k = 1.0f - q * q;
                    if (k <= 0.0f) continue;
                    att = k * k;
                }
            }

            const float ndl = Nx*lx + Ny*ly + Nz*lz;
            if (ndl <= 0.0f) continue;

            dr += li.r * att * ndl;
            dg += li.g * att * ndl;
            db += li.b * att * ndl;

            if (L.specular > 0.0f)
            {
                const float hx = lx + Vx, hy = ly + Vy, hz = lz + Vz;
                const float hlen = std::sqrt(hx*hx + hy*hy + hz*hz);
                if (hlen > 1e-8f)
                {
                    const float ndh = (Nx*hx + Ny*hy + Nz*hz) / hlen;
                    if (ndh > 0.0f)
                    {
                        const float s = std::pow(ndh, L.shininess) * att * L.specular;
                        sr += li.r * s; sg += li.g * s; sb += li.b * s;
                    }
                }
            }
        }

        const float* base = &baseColor[v * stride];
        lit[v * 3 + 0] = std::clamp(base[0] * dr + sr, 0.0f, 1.0f);
        lit[v * 3 + 1] = std::clamp(base[1] * dg + sg, 0.0f, 1.0f);
        lit[v * 3 + 2] = std::clamp(base[2] * db + sb, 0.0f, 1.0f);
    };

    // ------------------------------------------------------------
    // Main loop
    // ------------------------------------------------------------
    const auto startTime = std::chrono::steady_clock::now();
    double lastTime = 0.0;

    CamState lastCam{};
    bool haveCam = false;

    while (sgl_running())
    {
        const double elapsed =
            std::chrono::duration<double>(std::chrono::steady_clock::now() - startTime).count();
        const double dt = elapsed - lastTime;
        lastTime = elapsed;

        // Let the application move the camera / rotate objects.
        if (frameCallback)
            frameCallback(mapr, dt, elapsed);

        bool objectsChanged = false;
        for (size_t o = 0; o < mapr.objects.size(); ++o)
        {
            const ObjState st = make_state(mapr.objects[o]);
            if (!(st == objState[o]))
            {
                objState[o] = st;
                apply_object(o);
                objectsChanged = true;
            }
        }

        const Camera& cam = mapr.camera;
        const CamState cs{cam.x, cam.y, cam.z, cam.dir, cam.pitch, cam.roll, cam.fov};
        if (!haveCam || !(cs == lastCam) || objectsChanged)
        {
            lastCam = cs;
            haveCam = true;
            update_projection();
            std::fill(valid.begin(), valid.end(), 0);
        }

        eyeX = cs.x; eyeY = cs.y; eyeZ = cs.z;
        camR = euler_matrix(rad(cam.dir), rad(cam.pitch), rad(cam.roll));

        size_t slot = 0;
        if (animate)
            slot = static_cast<size_t>(elapsed * fps) % slots;

        if (!valid[slot])
        {
            cache[slot] = animate ? build_list(slot, slot + 1)
                                  : build_list(0, mapr.objects.size());
            valid[slot] = 1;
        }

        FrameList& list = cache[slot];

        if (lighting)
        {
            update_lights(elapsed);

            if (list.litVersion != lightsVersion)
            {
                for (const int v : list.used)
                    light_vertex(static_cast<size_t>(v));

                for (auto& df : list.faces)
                {
                    for (size_t k = 0; k < df.idx.size(); ++k)
                    {
                        const size_t v = static_cast<size_t>(df.idx[k]);
                        df.c[k * stride + 0] = lit[v * 3 + 0];
                        df.c[k * stride + 1] = lit[v * 3 + 1];
                        df.c[k * stride + 2] = lit[v * 3 + 2];
                    }
                }
                list.litVersion = lightsVersion;
            }
        }

        sgl_begin(
            config.window.bgcolor[0],
            config.window.bgcolor[1],
            config.window.bgcolor[2]
        );

        for (const auto& df : list.faces)
            draw_poly(df.x, df.y, df.c);

        sgl_end();
    }

    sgl_quit();
    GC_Shutdown();
}
