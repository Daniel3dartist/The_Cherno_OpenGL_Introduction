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

### Build (summary)

```bash
scons                    # Linux + Windows (debug)
scons target=linux       # -> build/linux/helloworld
scons target=windows     # -> build/windows/helloworld.exe
scons mode=release       # optimized
```

### Documentation

- **[.doc/SETUP.md](.doc/SETUP.md)** — prerequisites, how to open in the devcontainer, how
  the GPU/rendering reaches Windows, how to build, and how to debug the window.
- **[.doc/HELLOWORLD.md](.doc/HELLOWORLD.md)** — test triangle ([`helloworld/`](helloworld/))
  to validate the setup, the GPU, and the screen stream.

---

## License

See [LICENSE](LICENSE).
