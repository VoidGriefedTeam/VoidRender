/* simplegl.h - public C++ API for the simplegl DLL
 *
 * The draw functions take std::vector<float> directly. Because STL types
 * cross the DLL boundary, build your program with the same compiler and
 * CRT/runtime settings as simplegl.dll (e.g. both MSVC /MD).
 * The window/frame/mode functions are plain C functions.
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

#ifndef __cplusplus
  #error "simplegl.h requires C++ (the draw API uses std::vector<float>)"
#endif

#include <vector>

extern "C" {

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

}  /* extern "C" */

/* ---- drawing (std::vector<float>) ----
 * x, y:    one float per vertex (separate vectors)
 * colors:  r,g,b per vertex, flat (0..1) -- size = 3 * vertex count
 * Sizes are validated; on a mismatch nothing is drawn and sgl_error()
 * is set. */
SGL_API void draw_tri (const std::vector<float>& x, const std::vector<float>& y,
                       const std::vector<float>& colors);   /* 3 verts */
SGL_API void draw_rect(const std::vector<float>& x, const std::vector<float>& y,
                       const std::vector<float>& colors);   /* 4 verts */
/* n = x.size() (>= 3), convex */
SGL_API void draw_poly(const std::vector<float>& x, const std::vector<float>& y,
                       const std::vector<float>& colors);

/* Line: draw_line({x,y}, {x,y}, {r,g,b}, thickness in PIXELS (every
 * coordinate mode)). Flat butt ends, anti-aliased by MSAA. */
SGL_API void draw_line(const std::vector<float>& p0, const std::vector<float>& p1,
                       const std::vector<float>& rgb, float thickness);

#endif /* SIMPLEGL_H */
