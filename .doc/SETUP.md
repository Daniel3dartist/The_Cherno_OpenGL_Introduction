# Setup & Build

Guide to set up the environment, build (Linux + Windows), and open the OpenGL
rendering from inside the container on Windows to debug the window.

---

## 1. Host prerequisites (Windows)

| Component | Notes |
|---|---|
| Windows 11 | with **WSL2** enabled (`wsl --update`) |
| WSL2 distro | any (e.g. Ubuntu/Debian) — provides **WSLg** and `/dev/dxg` |
| GPU driver | vendor driver installed **on Windows** (NVIDIA/AMD/Intel). This is what WSL shares |
| Podman | installed **inside WSL2** |
| VS Code | with the **Dev Containers** extension (`ms-vscode-remote.remote-containers`) |

### Make VS Code use Podman
In the VS Code *Settings* (WSL / Remote side), set:

```jsonc
"dev.containers.dockerPath": "podman"
```

> The container runs as **root** (project requirement). With *rootless* Podman, the
> container root is mapped to your WSL user — enough to access `/dev/dxg`
> (usually `0666`) and write to the workspace bind. If the device is missing, run
> Podman *rootful* (`sudo podman ...`) or adjust the `/dev/dxg` permissions.

---

## 2. Open in the devcontainer

1. Clone the project on Windows (e.g. `C:\Users\<you>\Documents\projects\The_Cherno_OpenGL_Introduction`).
2. Open the folder in VS Code.
3. `F1` → **Dev Containers: Reopen in Container**.
4. The first creation **builds GLFW from source** (Linux + Windows) — it takes a while,
   but becomes a cached layer; recreations reuse it.

### Workspace bind
The [devcontainer.json](../.devcontainer/devcontainer.json) mounts the local workspace
folder into `/workspace`:

```jsonc
"workspaceMount": "source=${localWorkspaceFolder},target=/workspace,type=bind,consistency=cached",
"workspaceFolder": "/workspace"
```

## 3. GPU and rendering on Windows (how it works)

On Windows + WSL2 there is **no** GPU "with zero virtualization": the real path is the
host GPU **shared through the WSL compute layer** (GPU-PV) — **no** dedicated passthrough
and **no** software render. That is what the devcontainer configures:

- `--device=/dev/dxg` → access to the host GPU.
- mount `/usr/lib/wsl` + `LD_LIBRARY_PATH=/usr/lib/wsl/lib` → the WSL GPU loader.
- **Mesa** with the **d3d12** driver → translates OpenGL to the GPU via D3D12. That is why
  it works with **any vendor** (NVIDIA/AMD/Intel) and **any GPU** (onboard or offboard):
  detection is automatic, nothing is hardcoded for the RTX 5060.
- mounts for `/mnt/wslg` + `/tmp/.X11-unix` and `DISPLAY=:0` → **WSLg** shows the window
  directly on Windows (no extra X server like VcXsrv).

### Validate the GPU and the display
Inside the container:

```bash
glxinfo -B      # "OpenGL renderer string" should show your GPU (e.g. NVIDIA ...),
                # and NOT "llvmpipe" (which would be software render)
glxgears        # should open a window with gears on your Windows desktop
```

If you see `llvmpipe`: `/dev/dxg`, the `/usr/lib/wsl` mount, or `LD_LIBRARY_PATH` is
missing — check the [devcontainer.json](../.devcontainer/devcontainer.json).
If no window opens: check `DISPLAY=:0` and the WSLg mounts.

---

## 4. Build (SCons)

| Command | Result |
|---|---|
| `scons` | builds Linux **and** Windows (debug) |
| `scons target=linux` | Linux only → `build/linux/helloworld` |
| `scons target=windows` | Windows only → `build/windows/helloworld.exe` |
| `scons mode=release` | optimized (`-O2`) |
| `scons -c` | cleans the artifacts |

- **Linux:** links GLFW installed in `/usr/local` + GLEW/GL via `pkg-config` (static).
- **Windows:** cross-compiled with `x86_64-w64-mingw32-g++`, GLFW from
  `/opt/glfw/build-windows`, GLEW compiled from source (`glew.c`, `GLEW_STATIC`).
  The `.exe` is **self-contained** (`-static`), with no DLL dependencies.

---

## 5. Run

### Linux (inside the container, window on Windows via WSLg)
```bash
scons target=linux
./build/linux/helloworld
```

### Windows (outside the container)
`build/windows/helloworld.exe` lives in the workspace bind, so it shows up on Windows at
`...\The_Cherno_OpenGL_Introduction\build\windows\`. Open it in a **second window**
(Explorer or another VS Code window on Windows) and run it directly.

---

## 6. Use a custom GLFW fork
The container clones GLFW at creation time. To point it at your fork, edit the `build.args`
in [devcontainer.json](../.devcontainer/devcontainer.json) and rebuild the container:

```jsonc
"args": {
  "GLFW_REPO": "https://github.com/<you>/glfw.git",
  "GLFW_REF": "my-branch"   // or a tag, e.g. "3.4"
}
```

---

## 7. Troubleshooting

| Symptom | Likely cause / fix |
|---|---|
| `glxinfo` shows `llvmpipe` | missing `/dev/dxg`, `/usr/lib/wsl`, or `LD_LIBRARY_PATH` |
| Window does not open | wrong `DISPLAY` or missing WSLg mounts |
| `permission denied` on `/dev/dxg` | rootless Podman without access — run rootful or fix permissions |
| Slow build / slow I/O | the workspace is a 9p bind (`/mnt/c`); the GLFW build lives in `/opt` (fast) |
| `pkg-config: glfw3 not found` | the image did not finish the GLFW `cmake --install` — rebuild the container |
