#include <render.hpp>

#include <iostream>
#include <vector>
#include <cmath>

using std::vector;

int main()
{
    std::cout << "[Test] Creating VoidRender...\n";

    Render renderer;

    std::cout << "[Test] Loading config...\n";
    renderer.load_config("config.json");

    Object cube;

    cube.x = {
        -1,  1,  1, -1,
        -1,  1,  1, -1
    };

    cube.y = {
        -1, -1,  1,  1,
        -1, -1,  1,  1
    };

    cube.z = {
        -1, -1, -1, -1,
         1,  1,  1,  1
    };

    cube.color = {
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1},
        {1, 1, 0},
        {1, 0, 1},
        {0, 1, 1},
        {1, 1, 1},
        {1, 0.5f, 0}
    };

    cube.faces = {
        {0, 1, 2, 3},
        {4, 7, 6, 5},
        {0, 4, 5, 1},
        {3, 2, 6, 7},
        {0, 3, 7, 4},
        {1, 5, 6, 2}
    };

    Map map;
    map.objects.push_back(cube);

    map.camera.x = 5;
    map.camera.y = 2;
    map.camera.z = 5;
    map.camera.fov = 90;
    map.camera.dir = 45;

    std::cout << "[Test] Updating camera...\n";

    renderer.update_cam(
        map.camera.x,
        map.camera.y,
        map.camera.z,
        map.camera,
        map.camera.dir,
        map.camera.fov
    );

    std::cout << "[Test] Starting render pipeline...\n";

    renderer.render_map(map);

    std::cout << "[Test] render_map returned.\n";
    return 0;
}
