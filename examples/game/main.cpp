#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <math.h>
#include <numbers>
#include <algorithm>
#include <render.hpp>

int main()
{
    Render render;
    render.load_config("smoke.config.json");

    Map map = render.load_map("smoke_anim.mapb");

    constexpr float moveSpeed = 5.0f;  // world units/second
    constexpr float turnSpeed = 100.0f; // degrees/second


    render.set_frame_callback(
        Render::FrameCallback{
            [](Map& map, double dt, double time) -> void
            {
                Camera& cam = map.camera;

            const float move =
                moveSpeed * static_cast<float>(dt);
            const float turn =
                turnSpeed * static_cast<float>(dt);

            // Arrow keys: rotate camera horizontally.
            if (GetAsyncKeyState(VK_LEFT) & 0x8000)
                cam.dir += turn;

            if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
                cam.dir -= turn;

            if (GetAsyncKeyState(VK_UP) & 0x8000)
                cam.pitch += turn;

            if (GetAsyncKeyState(VK_DOWN) & 0x8000)
                cam.pitch -= turn;

            if (GetAsyncKeyState('E') & 0x8000)
                cam.roll -= turn;
                
            if (GetAsyncKeyState('Q') & 0x8000)
                cam.roll += turn;
                
            // Keep heading within 0–360 degrees.
            cam.dir = std::fmod(cam.dir, 360.0f);
            if (cam.dir < 0.0f)
                cam.dir += 360.0f;

            const float yaw =
                cam.dir * std::numbers::pi_v<float> / 180.0f;

            const float forwardX = -std::sin(yaw);
            const float forwardZ = -std::cos(yaw);

            // WASD: move relative to where the camera faces.
            if (GetAsyncKeyState('W') & 0x8000)
            {
                cam.x += forwardX * move;
                cam.z += forwardZ * move;
            }

            if (GetAsyncKeyState('S') & 0x8000)
            {
                cam.x -= forwardX * move;
                cam.z -= forwardZ * move;
            }

            if (GetAsyncKeyState('A') & 0x8000)
            {
                cam.x -= std::cos(yaw) * move;
                cam.z += std::sin(yaw) * move;
            }

            if (GetAsyncKeyState('D') & 0x8000)
            {
                cam.x += std::cos(yaw) * move;
                cam.z -= std::sin(yaw) * move;
            }
            }
        }
    );

    map.camera.dir = 45;
    map.camera.fov = 110;

        render.update_cam(
            map.camera.x,
            map.camera.y,
            map.camera.z,
            map.camera,
            map.camera.dir,
            map.camera.fov
        );
    render.render_map(map);
    return 0;
}