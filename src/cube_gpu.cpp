#include <SimpleGL.h>
#include <VoidCLcompute.h>

#include <cmath>
#include <cstdio>
#include <vector>
#include <print>

struct Object
{
    std::vector<float> x;
    std::vector<float> y;
    std::vector<float> z;
};

int screenWidth = 640;
int screenHeight = 360;

float camX = 0.0f;
float camY = 3.0f;
float camZ = -65.0f;

int main()
{
    GC_Init();

    sgl_init(screenWidth, screenHeight, "VoidRender");

    Object cube;

    cube.x =
    {
        -1,  1,  1, -1,
        -1,  1,  1, -1
    };

    cube.y =
    {
        -1, -1, -1, -1,
         1,  1,  1,  1
    };

    cube.z =
    {
         1,  1, -1, -1,
         1,  1, -1, -1
    };

    
}