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
# Linux builds also write compile_commands.json at the project root (used by
# clangd for include paths/defines; only Linux commands are recorded).
#
# Output:
#   build/linux/<app>          (ELF, runs inside the container via WSLg)
#   build/windows/<app>.exe    (self-contained PE, runs on Windows outside the container)
#
# Dependencies (static, copied into the project by the devcontainer):
#   dependencies/linux/{include,lib}    -> libglfw3.a, libGLEW.a
#   dependencies/windows/{include,lib}  -> libglfw3.a, libglew32.a
# Only the OS-provided OpenGL/system libraries stay dynamic (libGL/libc on Linux,
# opengl32/gdi32 on Windows).
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

# Dependency paths (inside the project)
DEPS_DIR = Dir("#dependencies").abspath
DEPS_LINUX = os.path.join(DEPS_DIR, "linux")
DEPS_WINDOWS = os.path.join(DEPS_DIR, "windows")

# =============================================================================
# Linux environment (native g++)
# =============================================================================
env_linux = None
if build_linux:
    env_linux = Environment(
        ENV=os.environ,
        CXX="g++",
        CXXFLAGS=common_cxxflags,
        CPPDEFINES=["GLEW_STATIC"],
        CPPPATH=[os.path.join(DEPS_LINUX, "include")],
        LIBPATH=[os.path.join(DEPS_LINUX, "lib")],
        # Full paths to the archives: guarantees the static .a is used even if a
        # system .so with the same name exists. GLFW loads X11/Wayland via dlopen.
        LIBS=[
            File(os.path.join(DEPS_LINUX, "lib", "libglfw3.a")),
            File(os.path.join(DEPS_LINUX, "lib", "libGLEW.a")),
            "GL", "rt", "m", "dl", "pthread",
        ],
    )
    env_linux["PLATFORM_NAME"] = "linux"
    env_linux["BIN_SUFFIX"] = ""

    # Compilation database for clangd. The tool is loaded only in the Linux env,
    # so MinGW commands never end up in it.
    env_linux["COMPILATIONDB_USE_ABSPATH"] = True
    env_linux.Tool("compilation_db")
    env_linux.CompilationDatabase("#compile_commands.json")

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
        # GLEW_STATIC: static libglew32.a (no GLEW DLL).
        CPPDEFINES=["GLEW_STATIC"],
        CPPPATH=[os.path.join(DEPS_WINDOWS, "include")],
        LIBPATH=[os.path.join(DEPS_WINDOWS, "lib")],
        # Self-contained .exe (no dependency on the GCC runtime DLLs).
        LINKFLAGS=["-static", "-static-libgcc", "-static-libstdc++"],
        # Order matters on MinGW: glfw3/glew32 -> opengl32/gdi32.
        LIBS=["glfw3", "glew32", "opengl32", "gdi32"],
        PROGSUFFIX=".exe",
    )
    env_windows["PLATFORM_NAME"] = "windows"
    env_windows["BIN_SUFFIX"] = ".exe"

# =============================================================================
# Sub-builds
# =============================================================================
Export("env_linux", "env_windows", "build_linux", "build_windows")
SConscript("helloworld/SConscript")
SConscript("src/SConscript")
