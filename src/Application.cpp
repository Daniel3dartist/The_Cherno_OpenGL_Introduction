//#include <iostream>

#include <GL/gl.h>
#include <GLFW/glfw3.h>

int main(void)
{
    // Force the X11 backend on Linux: GLEW 2.2 loads functions through GLX and
    // fails on Wayland/EGL contexts (glewInit -> GLEW_ERROR_NO_GLX_DISPLAY).
    // Works on both hosts (WSLg and Linux desktops provide X11/XWayland).
    // No effect on Windows (Win32 backend).
    #if defined(__linux__)
        glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    #endif

    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit()) return -1;


    /* Create a windowed mode window and its OpenGL context */
    int width = 640;
    int height = 480;
    const char* title = "Hello World";
    window = glfwCreateWindow(width, height, title, NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_TRIANGLES);
        glVertex2f(-0.5f,-0.5f);
        glVertex2f(0.0f, 0.5f);
        glVertex2f(0.5f, -0.5f);
        glEnd();


        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process envents */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;

}