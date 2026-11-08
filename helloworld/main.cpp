// =============================================================================
// OpenGL hello world: a colored triangle.
// Validates the devcontainer setup, GPU acceleration, and the screen stream
// (WSLg) to Windows. GLFW + GLEW, OpenGL 3.3 core, VAO/VBO, inline shaders.
// =============================================================================
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <cstdio>
#include <cstdlib>

// --- Shaders -----------------------------------------------------------------
static const char* kVertexShader = R"(#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aColor;
out vec3 vColor;
void main() {
    vColor = aColor;
    gl_Position = vec4(aPos, 1.0);
}
)";

static const char* kFragmentShader = R"(#version 330 core
in vec3 vColor;
out vec4 FragColor;
void main() {
    FragColor = vec4(vColor, 1.0);
}
)";

// --- Helpers -----------------------------------------------------------------
static void glfwErrorCallback(int code, const char* desc) {
    std::fprintf(stderr, "[GLFW] error %d: %s\n", code, desc);
}

static void framebufferSizeCallback(GLFWwindow*, int width, int height) {
    glViewport(0, 0, width, height);
}

static GLuint compileShader(GLenum type, const char* src) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    GLint ok = GL_FALSE;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &ok);
    if (ok != GL_TRUE) {
        char log[1024];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[GL] shader compilation failed: %s\n", log);
    }
    return shader;
}

static GLuint buildProgram() {
    GLuint vs = compileShader(GL_VERTEX_SHADER, kVertexShader);
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, kFragmentShader);

    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);

    GLint ok = GL_FALSE;
    glGetProgramiv(program, GL_LINK_STATUS, &ok);
    if (ok != GL_TRUE) {
        char log[1024];
        glGetProgramInfoLog(program, sizeof(log), nullptr, log);
        std::fprintf(stderr, "[GL] program linking failed: %s\n", log);
    }

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

int main() {
    glfwSetErrorCallback(glfwErrorCallback);

    // Force the X11 backend on Linux: GLEW 2.2 loads functions through GLX and
    // fails on Wayland/EGL contexts (glewInit -> GLEW_ERROR_NO_GLX_DISPLAY).
    // Works on both hosts (WSLg and Linux desktops provide X11/XWayland).
    // No effect on Windows (Win32 backend).
#if defined(__linux__)
    glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
#endif

    if (!glfwInit()) {
        std::fprintf(stderr, "Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    // OpenGL 3.3 core
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    GLFWwindow* window = glfwCreateWindow(800, 600, "OpenGL Hello Triangle", nullptr, nullptr);
    if (!window) {
        std::fprintf(stderr, "Failed to create the GLFW window\n");
        glfwTerminate();
        return EXIT_FAILURE;
    }
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);  // vsync
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);

    // GLEW needs a current context. experimental = compatible with core profile.
    glewExperimental = GL_TRUE;
    GLenum glewErr = glewInit();
    if (glewErr != GLEW_OK) {
        std::fprintf(stderr, "Failed to initialize GLEW: %s\n",
                     glewGetErrorString(glewErr));
        glfwDestroyWindow(window);
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // Confirm which GPU/driver was selected (host via WSL, not llvmpipe).
    std::printf("GL_VENDOR   : %s\n", glGetString(GL_VENDOR));
    std::printf("GL_RENDERER : %s\n", glGetString(GL_RENDERER));
    std::printf("GL_VERSION  : %s\n", glGetString(GL_VERSION));

    // Geometry: 3 vertices with position (xyz) + color (rgb).
    const float vertices[] = {
        //   position          color
        -0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,
    };

    GLuint vao = 0, vbo = 0;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // location 0: position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)0);
    glEnableVertexAttribArray(0);
    // location 1: color
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float),
                          (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    GLuint program = buildProgram();

    // Render loop
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(program);
        glBindVertexArray(vao);
        glDrawArrays(GL_TRIANGLES, 0, 3);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
