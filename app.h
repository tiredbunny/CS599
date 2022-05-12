
#include "camera.h"

class App
{
public:
    GLFWwindow* GLFW_window;
    App(int argc, char** argv);
    bool doApiDump;
    
    Camera myCamera;
    void updateCamera();
};

inline void throwIfFailed(VkResult v)
{
    if (v != VK_SUCCESS)
        throw std::exception();
}