# =============================================================================
# SConstruct - cross-platform build (native Linux + Windows cross-compile MinGW)
#
# Usage (run "scons help" for the full list):
#   scons help                 -> show the commands (same as scons -h)
#   scons                      -> build for Linux and Windows (default), debug mode
#   scons target=linux         -> Linux only
#   scons target=windows       -> Windows only (.exe)
#   scons mode=release         -> optimized
#   scons --helloworld         -> build ONLY the helloworld test (instead of the app)
#   scons -r | --run           -> after the build, run the binary for this OS
#   scons -c                   -> clean (app and helloworld)
#
#   Options combine, e.g.: scons target=linux --helloworld -r
#
# What gets built: by default only the application (src/ -> app). With
# --helloworld only the helloworld test is built.
#
# --run executes the binary that matches the OS SCons is running on. Inside the
# devcontainer that is always Linux: target=windows prints a warning (a .exe
# cannot run here) and target=all runs only the Linux binary.
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
import platform
import sys

# --- Command-line options ----------------------------------------------------
AddOption(
    "--helloworld",
    dest="helloworld",
    action="store_true",
    default=False,
    help="build only the helloworld test instead of the application",
)
AddOption(
    "--run",
    dest="run",
    action="store_true",
    default=False,
    help="run the built binary (for the current OS) after the build (alias: -r)",
)

HELP_TEXT = """
Usage: scons [VARIABLES] [OPTIONS]

Commands:
  scons help                 show this help (same as: scons -h)
  scons                      build the application (src/) for Linux + Windows, debug
  scons -c                   clean everything (app and helloworld)

Variables (name=value):
  target=all|linux|windows   platform(s) to build              (default: all)
  mode=debug|release         debug (-g -O0) or optimized (-O2)  (default: debug)

Options:
  --helloworld               build ONLY the helloworld test instead of the application
  -r, --run                  after the build, run the binary for the current OS
                             (inside the container: the Linux one; a .exe is not run)
  -j N                       build with N parallel jobs, e.g. -j$(nproc)

Examples:
  scons target=linux -r                   build + run the app on Linux
  scons target=linux --helloworld -r      build + run the helloworld test
  scons target=windows mode=release       final Windows build -> build/windows/app.exe
  scons target=linux -j$(nproc)           Linux only, parallel

Output:
  build/linux/app, build/linux/helloworld
  build/windows/app.exe, build/windows/helloworld.exe

Notes:
  -r prints "Warning: the -r option is not yet implemented" (SCons reserves the
  name). It still works; use --run to avoid the message.
  Full guide: .doc/SETUP.md
"""
# Shown by "scons -h" (replaces SCons' auto-generated local options list).
Help(HELP_TEXT)

HELP_HINT = "Run 'scons help' to see the list of commands."


def usage_error(msg):
    print("scons: error: %s\n%s" % (msg, HELP_HINT))
    Exit(2)


# "scons help": print the help and stop before configuring anything.
if "help" in COMMAND_LINE_TARGETS:
    print(HELP_TEXT)
    Exit(0)

# Unknown --options / -x options. SCons only rejects them after reading this
# file, with a generic message: check here first to point at "scons help".
from SCons.Script.Main import OptionsParser  # noqa: E402

for arg in sys.argv[1:]:
    name = arg.split("=", 1)[0]
    is_long = name.startswith("--") and len(name) > 2
    is_short = len(name) == 2 and name[0] == "-" and name[1] != "-"
    if (is_long or is_short) and not OptionsParser.has_option(name):
        usage_error("unknown option '%s'." % name)

KNOWN_VARIABLES = ("target", "mode")
for key in ARGUMENTS:
    if key not in KNOWN_VARIABLES:
        usage_error("unknown variable '%s' (valid: %s)." % (key, ", ".join(KNOWN_VARIABLES)))

target = ARGUMENTS.get("target", "all").lower()
mode = ARGUMENTS.get("mode", "debug").lower()
only_helloworld = GetOption("helloworld")
# "-r" is reserved by SCons itself (a legacy option it accepts and ignores), so
# AddOption cannot register it: read it straight from the command line instead.
run_after_build = GetOption("run") or "-r" in sys.argv[1:]
cleaning = GetOption("clean")

if target not in ("all", "linux", "windows"):
    usage_error("invalid target '%s': use linux | windows | all." % target)
if mode not in ("debug", "release"):
    usage_error("invalid mode '%s': use debug | release." % mode)

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
compile_db = None
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
    compile_db = env_linux.CompilationDatabase("#compile_commands.json")

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
# Each SConscript returns {"linux": node | None, "windows": node | None}.
Export("env_linux", "env_windows", "build_linux", "build_windows")

programs = {}
if only_helloworld or cleaning:
    programs["helloworld"] = SConscript("helloworld/SConscript")
if not only_helloworld or cleaning:
    programs["app"] = SConscript("src/SConscript")

# Only build what was selected (scons -c still cleans everything).
if compile_db is not None:
    Default(compile_db)
for prog in programs.values():
    for node in prog.values():
        if node is not None:
            Default(node)

# Unknown command-line targets ("scons foo"): SCons would stop with "Do not
# know how to make File target"; say what happened and point at "scons help".
import SCons.Node.Alias  # noqa: E402

for t in COMMAND_LINE_TARGETS:
    if SCons.Node.Alias.default_ans.lookup(t) is not None:
        continue
    node = Entry(t)
    if not (node.has_builder() or node.exists()):
        usage_error("unknown command or target '%s'." % t)

# =============================================================================
# --run
# =============================================================================
def host_is_windows_behind_container():
    """True when this Linux is WSL2 (directly or inside a Podman/Docker container
    on WSL): the kernel is shared, so its release string names Microsoft."""
    return "microsoft" in platform.release().lower() or os.path.exists("/dev/dxg")


def warn_exe_on_linux(exe_path):
    windows_host = host_is_windows_behind_container()
    lines = [
        "",
        "WARNING: not running '%s'." % exe_path,
        "  This is a Windows executable and SCons is running on Linux, the wrong OS for it.",
        "  To build and run here, use target=linux, e.g.:",
        "      scons target=linux%s -r" % (" --helloworld" if only_helloworld else ""),
    ]
    if windows_host:
        lines += [
            "  Your container runs on a Windows host (WSL2): even when compiled for Linux,",
            "  the window is shown on your Windows desktop (WSLg).",
        ]
    lines += [
        "  The .exe is recommended when developing directly on Windows, or for a final",
        "  Windows build: run it on Windows, outside the container.",
        "",
    ]
    print("\n".join(lines))


if run_after_build and not cleaning:
    app_name = "helloworld" if only_helloworld else "app"
    prog = programs[app_name]
    host = "windows" if sys.platform == "win32" else "linux"
    other = "linux" if host == "windows" else "windows"

    if prog.get(host) is not None:
        node = prog[host]
        if prog.get(other) is not None:
            print("note: --run executes only the %s binary (%s)." % (host, node[0].path))
        env_run = env_linux if host == "linux" else env_windows
        run = env_run.Command(
            "run-" + app_name,
            node,
            # cwd = project root, so relative asset paths work like in launch.json.
            Action('cd "%s" && "%s"' % (Dir("#").abspath, node[0].abspath),
                   "Running %s ..." % node[0].path),
        )
        AlwaysBuild(run)
        Default(run)
    elif host == "linux":
        warn_exe_on_linux(prog["windows"][0].path)
    else:
        print("\nWARNING: a Linux binary cannot run on Windows; use target=windows -r.\n")
