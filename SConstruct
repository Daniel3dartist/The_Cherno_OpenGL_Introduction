# =============================================================================
# SConstruct - build multiplataforma (Linux nativo + Windows cross-compile MinGW)
#
# Uso:
#   scons                      -> compila para Linux e Windows (default), modo debug
#   scons target=linux         -> so Linux
#   scons target=windows       -> so Windows (.exe)
#   scons mode=release         -> otimizado
#   scons -c                   -> limpa
#
# Saida:
#   build/linux/<app>          (ELF, roda dentro do container via WSLg)
#   build/windows/<app>.exe    (PE self-contained, roda no Windows fora do container)
#
# Dependencias (providas pelo devcontainer):
#   GLFW  -> compilado do fonte em /opt/glfw (Linux instalado em /usr/local; Windows
#            em /opt/glfw/build-windows/src/libglfw3.a)
#   GLEW  -> Linux: pacote libglew-dev; Windows: fonte /opt/glew (glew.c compilado junto)
# =============================================================================
import os

# --- Opcoes de linha de comando ----------------------------------------------
target = ARGUMENTS.get("target", "all").lower()
mode = ARGUMENTS.get("mode", "debug").lower()

if target not in ("all", "linux", "windows"):
    print("target invalido: use linux | windows | all")
    Exit(1)
if mode not in ("debug", "release"):
    print("mode invalido: use debug | release")
    Exit(1)

build_linux = target in ("all", "linux")
build_windows = target in ("all", "windows")

# Flags por modo
if mode == "debug":
    common_cxxflags = ["-g", "-O0", "-DDEBUG"]
else:
    common_cxxflags = ["-O2", "-DNDEBUG"]
common_cxxflags += ["-std=c++17", "-Wall", "-Wextra"]

# Caminhos das dependencias dentro do container
GLFW_INCLUDE = "/opt/glfw/src/include"
GLFW_WIN_LIBDIR = "/opt/glfw/build-windows/src"
GLEW_DIR = "/opt/glew"

# =============================================================================
# Ambiente Linux (g++ nativo)
# =============================================================================
env_linux = None
if build_linux:
    env_linux = Environment(
        ENV=os.environ,
        CXX="g++",
        CXXFLAGS=common_cxxflags,
        CPPPATH=[GLFW_INCLUDE],
    )
    # GLFW instalado em /usr/local + GLEW/GL via pkg-config (link estatico do GLFW
    # puxa as deps privadas: X11/Wayland/dl/pthread).
    env_linux.ParseConfig("pkg-config --static --cflags --libs glfw3 glew gl")
    env_linux.Append(LIBS=["dl", "pthread", "m"])
    env_linux["PLATFORM_NAME"] = "linux"
    env_linux["BIN_SUFFIX"] = ""

# =============================================================================
# Ambiente Windows (cross-compile MinGW-w64)
# =============================================================================
env_windows = None
if build_windows:
    env_windows = Environment(
        ENV=os.environ,
        CC="x86_64-w64-mingw32-gcc",
        CXX="x86_64-w64-mingw32-g++",
        LINK="x86_64-w64-mingw32-g++",
        CXXFLAGS=common_cxxflags,
        # GLEW_STATIC: glew.c compilado junto do app (sem DLL do GLEW).
        CPPDEFINES=["GLEW_STATIC"],
        CPPPATH=[GLFW_INCLUDE, os.path.join(GLEW_DIR, "include")],
        LIBPATH=[GLFW_WIN_LIBDIR],
        # .exe self-contained (sem depender de DLLs do runtime GCC).
        LINKFLAGS=["-static", "-static-libgcc", "-static-libstdc++"],
        # Ordem importa no MinGW: glfw3 -> opengl32/gdi32.
        LIBS=["glfw3", "opengl32", "gdi32"],
        PROGSUFFIX=".exe",
    )
    # Fonte amalgamado do GLEW compilado junto (como C).
    env_windows["GLEW_C"] = os.path.join(GLEW_DIR, "src", "glew.c")
    env_windows["PLATFORM_NAME"] = "windows"
    env_windows["BIN_SUFFIX"] = ".exe"

# =============================================================================
# Sub-builds
# =============================================================================
Export("env_linux", "env_windows", "build_linux", "build_windows")
SConscript("helloworld/SConscript")
