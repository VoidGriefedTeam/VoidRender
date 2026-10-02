/* simplegl.h - public API for the simplegl DLL
 *
 * The DLL exports a plain C ABI (pointers only), so it is safe across
 * compilers/CRTs. In C++ this header additionally provides inline
 * std::vector<float> overloads; they are compiled into YOUR program and
 * only pass raw pointers across the DLL boundary (no STL types cross it).
 *
 * Coordinates default to screen pixels: (0,0) top-left, +y down
 * (see sgl_set_center_mode / sgl_set_clip_mode for the others).
 * Colors are per-vertex RGB (0..1), interpolated across the fill,
 * with edges anti-aliased via 8x MSAA.
 *
 * Link against simplegl.lib and put simplegl.dll next to your exe.
 */

#ifndef SIMPLEGL_H
#define SIMPLEGL_H

#if defined(_WIN32)
  #ifdef SIMPLEGL_BUILD_DLL
    #define SGL_API __declspec(dllexport)
  #else
    #define SGL_API __declspec(dllimport)
  #endif
#else
  #define SGL_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ---- window / frame ---- */
SGL_API int  sgl_init(int width, int height, const char* title); /* 1 = ok, 0 = fail */
SGL_API int  sgl_running(void);                                   /* 0 when closed or not initialised */
SGL_API void sgl_begin(float r, float g, float b);                /* clear frame */
SGL_API void sgl_end(void);                                       /* flush + swap + poll */
SGL_API void sgl_quit(void);
SGL_API const char* sgl_error(void);                              /* last error message */
SGL_API void sgl_report_error(const char* msg);                   /* set + print an error */

/* Coordinate mode.
 * Pixel mode (default): pixels, (0,0) top-left, +y down.
 * Center mode:          pixels, (0,0) window centre, +y up
 *                       (x: -width/2..width/2, y: -height/2..height/2).
 * Clip mode:            raw -1..1, (0,0) centre, +y up. */
SGL_API void sgl_set_pixel_mode(void);
SGL_API void sgl_set_center_mode(void);
SGL_API void sgl_set_clip_mode(void);

/* Windows are fixed-size by default. Call this (before or after sgl_init)
 * to make the window resizable/maximizable. */
SGL_API void sgl_resize_mode(void);

/* ---- drawing (C ABI, raw pointers) ----
 * x, y:    one float per vertex (separate arrays)
 * colors:  r,g,b per vertex, flat array (0..1) -- length = 3 * vertex_count
 */
SGL_API void draw_tri (const float* x, const float* y, const float* colors); /* 3 verts */
SGL_API void draw_rect(const float* x, const float* y, const float* colors); /* 4 verts */
SGL_API void draw_poly(int n, const float* x, const float* y, const float* colors); /* n verts, convex */

/* Line: p0[2] = {x,y}, p1[2] = {x,y}, rgb[3] (0..1), thickness in PIXELS
 * (in every coordinate mode). Flat butt ends, anti-aliased by MSAA. */
SGL_API void draw_line(const float* p0, const float* p1, const float* rgb, float thickness);

#ifdef __cplusplus
}

/* ---- drawing (C++, std::vector<float>) ----
 * Sizes are validated; on a mismatch nothing is drawn and sgl_error()
 * is set. */
#include <cstddef>
#include <vector>

namespace sgl_detail {
inline bool check(const char* msg, std::size_t verts,
                  const std::vector<float>& x,
                  const std::vector<float>& y,
                  const std::vector<float>& c) {
    if (x.size() == verts && y.size() == verts && c.size() == verts * 3)
        return true;
    sgl_report_error(msg);
    return false;
}
} // namespace sgl_detail

inline void draw_tri(const std::vector<float>& x, const std::vector<float>& y,
                     const std::vector<float>& colors) {
    if (sgl_detail::check("draw_tri: need x[3], y[3], colors[9]", 3, x, y, colors))
        draw_tri(x.data(), y.data(), colors.data());
}

inline void draw_rect(const std::vector<float>& x, const std::vector<float>& y,
                      const std::vector<float>& colors) {
    if (sgl_detail::check("draw_rect: need x[4], y[4], colors[12]", 4, x, y, colors))
        draw_rect(x.data(), y.data(), colors.data());
}

/* n is the vertex count = x.size() (>= 3, convex). */
inline void draw_poly(const std::vector<float>& x, const std::vector<float>& y,
                      const std::vector<float>& colors) {
    if (x.size() < 3) { sgl_report_error("draw_poly: need at least 3 vertices"); return; }
    if (sgl_detail::check("draw_poly: need x[n], y[n], colors[3n]", x.size(), x, y, colors))
        draw_poly((int)x.size(), x.data(), y.data(), colors.data());
}

/* draw_line({x,y}, {x,y}, {r,g,b}, thickness_in_pixels) */
inline void draw_line(const std::vector<float>& p0, const std::vector<float>& p1,
                      const std::vector<float>& rgb, float thickness) {
    if (p0.size() != 2 || p1.size() != 2 || rgb.size() != 3) {
        sgl_report_error("draw_line: need {x,y}, {x,y}, {r,g,b}");
        return;
    }
    draw_line(p0.data(), p1.data(), rgb.data(), thickness);
}
#endif /* __cplusplus */

#endif /* SIMPLEGL_H */
