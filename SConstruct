# =============================================================================
# SConstruct - cross-platform build (native Linux + Windows cross-compile MinGW)
#
# Usage:
#   scons                      -> build for Linux and Windows (default), debug mode
#   scons target=linux         -> Linux only
#   scons target=windows       -> Windows only (.exe)
#   scons mode=release         -> optimized
#   scons -c                   -> clean
#
# Output:
#   build/linux/<app>          (ELF, runs inside the container via WSLg)
#   build/windows/<app>.exe    (self-contained PE, runs on Windows outside the container)
#
# Dependencies (provided by the devcontainer):
#   GLFW  -> built from source in /opt/glfw (Linux installed into /usr/local; Windows
#            in /opt/glfw/build-windows/src/libglfw3.a)
#   GLEW  -> Linux: libglew-dev package; Windows: /opt/glew source (glew.c compiled in)
# =============================================================================
import os

# --- Command-line options ----------------------------------------------------
target = ARGUMENTS.get("target", "all").lower()
mode = ARGUMENTS.get("mode", "debug").lower()

if target not in ("all", "linux", "windows"):
    print("invalid target: use linux | windows | all")
    Exit(1)
if mode not in ("debug", "release"):
    print("invalid mode: use debug | release")
    Exit(1)

build_linux = target in ("all", "linux")
build_windows = target in ("all", "windows")

# Per-mode flags
if mode == "debug":
    common_cxxflags = ["-g", "-O0", "-DDEBUG"]
else:
    common_cxxflags = ["-O2", "-DNDEBUG"]
common_cxxflags += ["-std=c++17", "-Wall", "-Wextra"]

# Dependency paths inside the container
GLFW_INCLUDE = "/opt/glfw/src/include"
GLFW_WIN_LIBDIR = "/opt/glfw/build-windows/src"
GLEW_DIR = "/opt/glew"

# =============================================================================
# Linux environment (native g++)
# =============================================================================
env_linux = None
if build_linux:
    env_linux = Environment(
        ENV=os.environ,
        CXX="g++",
        CXXFLAGS=common_cxxflags,
        CPPPATH=[GLFW_INCLUDE],
    )
    # GLFW installed in /usr/local + GLEW/GL via pkg-config (static GLFW linking
    # pulls its private deps: X11/Wayland/dl/pthread).
    env_linux.ParseConfig("pkg-config --static --cflags --libs glfw3 glew gl")
    env_linux.Append(LIBS=["dl", "pthread", "m"])
    env_linux["PLATFORM_NAME"] = "linux"
    env_linux["BIN_SUFFIX"] = ""

# =============================================================================
# Windows environment (MinGW-w64 cross-compile)
# =============================================================================
env_windows = None
if build_windows:
    env_windows = Environment(
        ENV=os.environ,
        CC="x86_64-w64-mingw32-gcc",
        CXX="x86_64-w64-mingw32-g++",
        LINK="x86_64-w64-mingw32-g++",
        CXXFLAGS=common_cxxflags,
        # GLEW_STATIC: glew.c compiled into the app (no GLEW DLL).
        CPPDEFINES=["GLEW_STATIC"],
        CPPPATH=[GLFW_INCLUDE, os.path.join(GLEW_DIR, "include")],
        LIBPATH=[GLFW_WIN_LIBDIR],
        # Self-contained .exe (no dependency on the GCC runtime DLLs).
        LINKFLAGS=["-static", "-static-libgcc", "-static-libstdc++"],
        # Order matters on MinGW: glfw3 -> opengl32/gdi32.
        LIBS=["glfw3", "opengl32", "gdi32"],
        PROGSUFFIX=".exe",
    )
    # GLEW amalgamated source compiled in (as C).
    env_windows["GLEW_C"] = os.path.join(GLEW_DIR, "src", "glew.c")
    env_windows["PLATFORM_NAME"] = "windows"
    env_windows["BIN_SUFFIX"] = ".exe"

# =============================================================================
# Sub-builds
# =============================================================================
Export("env_linux", "env_windows", "build_linux", "build_windows")
SConscript("helloworld/SConscript")
