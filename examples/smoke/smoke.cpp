#include <render.hpp>

#include <iostream>
#include <filesystem>
#include <exception>

int main()
{
    try
    {
        std::cout << "[Test] Working dir: " << std::filesystem::current_path() << "\n";

        std::cout << "[Test] Loading config...\n";
        render.load_config("smoke.config.json");

        const char* map_path = "smoke_anim.mapb";   // compiled from smoke_anim.map via mapc
        if (!std::filesystem::exists(map_path))
        {
            std::cout << "[Test] ERROR: " << map_path << " not found in the working dir.\n";
            std::cin.get();
            return 1;
        }

        std::cout << "[Test] Loading map...\n";
        Map map = render.load_map(map_path);

        std::cout << "[Test] objects (frames): " << map.objects.size() << "\n";
        if (map.objects.empty())
        {
            std::cout << "[Test] ERROR: map has no objects (load failed or stale .mapb).\n";
            std::cin.get();
            return 1;
        }

        // The .map format has no camera direction field, so set it here.
        map.camera.dir = 45;

        render.update_cam(
            map.camera.x,
            map.camera.y,
            map.camera.z,
            map.camera,
            map.camera.dir,
            map.camera.fov
        );

        std::cout << "[Test] Starting render pipeline...\n";
        render.render_map(map);
        std::cout << "[Test] render_map returned.\n";
    }
    catch (const std::exception& e)
    {
        std::cout << "[Test] EXCEPTION: " << e.what() << "\n";
    }
    catch (...)
    {
        std::cout << "[Test] UNKNOWN EXCEPTION\n";
    }

    std::cout << "[Test] Press Enter to exit.\n";
    std::cin.get();
    return 0;
}
