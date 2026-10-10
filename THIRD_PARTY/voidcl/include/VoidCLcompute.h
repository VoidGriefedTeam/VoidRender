// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef VOIDCLCOMPUTE_H
#define VOIDCLCOMPUTE_H

#include <vector>

#if defined(_WIN32)
    #ifdef VOIDCLCOMPUTE_EXPORTS
        #define VOIDCLCOMPUTE_API __declspec(dllexport)
    #else
        #define VOIDCLCOMPUTE_API __declspec(dllimport)
    #endif
#elif defined(__GNUC__) || defined(__clang__)
    #define VOIDCLCOMPUTE_API __attribute__((visibility("default")))
#else
    #define VOIDCLCOMPUTE_API
#endif

// Lifecycle (plain C ABI, no arrays involved).
extern "C" {
VOIDCLCOMPUTE_API bool GC_Init();
VOIDCLCOMPUTE_API void GC_Shutdown();

// Release cached device buffers without a full context teardown.
VOIDCLCOMPUTE_API void GC_TrimBufferCache();
}

// ------------------------------------------------------------
// Elementwise ops.
//
// Every op reads its inputs from std::vector<float> and writes into
// `result`, which is resized to match the input if it isn't already.
// For binary ops, `a` and `b` must be the same size (otherwise an error
// is printed and `result` is left untouched). Empty input is a no-op.
//
// NOTE: these functions take C++ types across the DLL boundary, so the
// DLL and its callers must be built with the same compiler, C++ standard
// library and build configuration (Debug vs Release). The CMake build in
// this repo guarantees that for the bundled benchmark.
// ------------------------------------------------------------

// array (op) array
VOIDCLCOMPUTE_API void gpu_add     (const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& result);
VOIDCLCOMPUTE_API void gpu_subtract(const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& result);
VOIDCLCOMPUTE_API void gpu_multiply(const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& result);
VOIDCLCOMPUTE_API void gpu_divide  (const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& result);

// array (op) single number — no second array needed. Same names as above;
// the compiler picks the overload from the type of the second argument.
VOIDCLCOMPUTE_API void gpu_add     (const std::vector<float>& a, float scalar, std::vector<float>& result);
VOIDCLCOMPUTE_API void gpu_subtract(const std::vector<float>& a, float scalar, std::vector<float>& result);
VOIDCLCOMPUTE_API void gpu_multiply(const std::vector<float>& a, float scalar, std::vector<float>& result);
VOIDCLCOMPUTE_API void gpu_divide  (const std::vector<float>& a, float scalar, std::vector<float>& result);

VOIDCLCOMPUTE_API void gpu_sin (const std::vector<float>& input, std::vector<float>& result);
VOIDCLCOMPUTE_API void gpu_cos (const std::vector<float>& input, std::vector<float>& result);
VOIDCLCOMPUTE_API void gpu_tan (const std::vector<float>& input, std::vector<float>& result);
VOIDCLCOMPUTE_API void gpu_asin(const std::vector<float>& input, std::vector<float>& result);
VOIDCLCOMPUTE_API void gpu_acos(const std::vector<float>& input, std::vector<float>& result);
VOIDCLCOMPUTE_API void gpu_atan(const std::vector<float>& input, std::vector<float>& result);

VOIDCLCOMPUTE_API void gpu_heavy(const std::vector<float>& a, const std::vector<float>& b, std::vector<float>& result);

// ------------------------------------------------------------
// Return-by-value API (preferred):
//
//     std::vector<float> c = gpu_add(a, b);
//     std::vector<float> d = gpu_multiply(c, 0.5f);
//     std::vector<float> s = gpu_sin(d);
//
// On error (size mismatch, empty/oversized input, OpenCL failure) the
// returned vector is empty. The out-parameter versions above still exist
// for hot loops where you want to reuse an existing vector's storage.
// ------------------------------------------------------------

// array (op) array
VOIDCLCOMPUTE_API std::vector<float> gpu_add     (const std::vector<float>& a, const std::vector<float>& b);
VOIDCLCOMPUTE_API std::vector<float> gpu_subtract(const std::vector<float>& a, const std::vector<float>& b);
VOIDCLCOMPUTE_API std::vector<float> gpu_multiply(const std::vector<float>& a, const std::vector<float>& b);
VOIDCLCOMPUTE_API std::vector<float> gpu_divide  (const std::vector<float>& a, const std::vector<float>& b);

// array (op) single number
VOIDCLCOMPUTE_API std::vector<float> gpu_add     (const std::vector<float>& a, float scalar);
VOIDCLCOMPUTE_API std::vector<float> gpu_subtract(const std::vector<float>& a, float scalar);
VOIDCLCOMPUTE_API std::vector<float> gpu_multiply(const std::vector<float>& a, float scalar);
VOIDCLCOMPUTE_API std::vector<float> gpu_divide  (const std::vector<float>& a, float scalar);

VOIDCLCOMPUTE_API std::vector<float> gpu_sin (const std::vector<float>& input);
VOIDCLCOMPUTE_API std::vector<float> gpu_cos (const std::vector<float>& input);
VOIDCLCOMPUTE_API std::vector<float> gpu_tan (const std::vector<float>& input);
VOIDCLCOMPUTE_API std::vector<float> gpu_asin(const std::vector<float>& input);
VOIDCLCOMPUTE_API std::vector<float> gpu_acos(const std::vector<float>& input);
VOIDCLCOMPUTE_API std::vector<float> gpu_atan(const std::vector<float>& input);

VOIDCLCOMPUTE_API std::vector<float> gpu_heavy(const std::vector<float>& a, const std::vector<float>& b);

#endif // VOIDCLCOMPUTE_H
