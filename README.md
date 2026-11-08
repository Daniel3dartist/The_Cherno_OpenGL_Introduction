# The_Cherno_OpenGL_Introduction

Project to follow [The Cherno](https://www.youtube.com/playlist?list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2)'s
**OpenGL** tutorial, using **GLFW** + **GLEW**. Development happens in a **devcontainer**
(Debian 13), with a **cross-platform** build (Linux + Windows) and GPU-accelerated OpenGL
rendering shown directly on Windows via WSLg.

---

## Development environment

Development is done inside a devcontainer. The container builds for Linux (runs inside the
container, window on Windows via WSLg) and cross-compiles a `.exe` for Windows (runs
natively, outside the container).

### Components and versions

| Component | Version / detail |
|---|---|
| Container OS | Debian **13 (trixie)** |
| Container user | **root** |
| OpenGL | **3.3 core** |
| GLFW | **built from source** in the container (upstream `master` ≈ 3.6-dev; configurable via `GLFW_REF`) |
| GLEW | **2.2.0** (Linux via `libglew-dev`; Windows built from source) |
| Linux compiler | `g++` (trixie) |
| Windows cross-compile | **MinGW-w64** (`x86_64-w64-mingw32-g++`) |
| Build system | **SCons** |
| GPU / OpenGL driver | **Mesa** (`d3d12` driver, via WSL `/dev/dxg`) |

### Host dependencies (Windows)

- **Windows 11** with **WSL2** (`wsl --update`) and a GPU driver installed on Windows
  (NVIDIA / AMD / Intel — detection is automatic, onboard or offboard).
- **Podman** installed inside WSL2.
- **VS Code** + the **Dev Containers** extension (set `"dev.containers.dockerPath": "podman"`).

### Which devcontainer to pick

When you run **Dev Containers: Reopen in Container**, VS Code asks you to choose between two
configurations. Pick the one that matches **the machine you are sitting at**, not the
container (the container is the same Debian 13 image in both cases):

| Your computer runs… | Pick | Why |
|---|---|---|
| **Windows 11** (project opened through WSL2) | **OpenGL dev (Windows / WSL)** | Gets the GPU through WSL (`/dev/dxg` + Mesa `d3d12`) and shows the window on your Windows desktop via WSLg. |
| **Linux** directly (Ubuntu, Fedora, Arch… with a desktop) | **OpenGL dev (Linux)** | Gets the GPU through `/dev/dri` (Mesa picks the native driver) and shows the window on your own X11/XWayland session. |

**Why two options?** The Dockerfile, the compilers, and the build are identical. What
changes is how the container reaches the **GPU** and the **screen**, and that depends on
the host: Windows exposes them through WSL-specific devices and sockets that don't exist
on a regular Linux machine, and vice versa. Picking the wrong one usually means the
container fails to start (missing `/dev/dxg` or `/mnt/wslg`) or OpenGL falls back to
software rendering (`llvmpipe`) with no window.

> Not sure? If you installed WSL and Podman following the list above, you want
> **Windows / WSL**. The Linux option needs one extra host step (`xhost +local:`) — see
> [.doc/SETUP.md](.doc/SETUP.md#2-open-in-the-devcontainer).

### Build (summary)

```bash
scons help                         # list every command / option (same as scons -h)
scons                              # app for Linux + Windows (debug)
scons target=linux                 # -> build/linux/app
scons target=windows               # -> build/windows/app.exe
scons mode=release                 # optimized
scons target=linux --helloworld    # only the helloworld test -> build/linux/helloworld
scons target=linux -r              # build, then run (-r or --run, works with --helloworld too)
```

`-r` / `--run` runs the Linux binary inside the container. The window still appears on
your Windows desktop when the container runs on WSL2. A `.exe` is not run there: you get a
warning to use `target=linux`, and you run the `.exe` on Windows yourself.

### Documentation

- **[.doc/SETUP.md](.doc/SETUP.md)** — prerequisites, how to open in the devcontainer, how
  the GPU/rendering reaches Windows, how to build, and how to debug the window.
- **[.doc/HELLOWORLD.md](.doc/HELLOWORLD.md)** — test triangle ([`helloworld/`](helloworld/))
  to validate the setup, the GPU, and the screen stream.

---

## License

See [LICENSE](LICENSE).
