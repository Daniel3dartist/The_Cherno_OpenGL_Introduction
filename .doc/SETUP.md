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
3. `F1` → **Dev Containers: Reopen in Container** and pick the configuration for your host:
   - **OpenGL dev (Windows / WSL)** → [.devcontainer/wsl/devcontainer.json](../.devcontainer/wsl/devcontainer.json)
   - **OpenGL dev (Linux)** → [.devcontainer/linux/devcontainer.json](../.devcontainer/linux/devcontainer.json)
     (run `xhost +local:` on the host once; see the comments in that file for
     rootless Podman, SELinux and NVIDIA proprietary drivers)

   Both share the same [Dockerfile](../.devcontainer/Dockerfile); only the host
   integration (GPU device, display sockets, env vars) differs.
4. The first creation **builds GLFW from source** (Linux + Windows) — it takes a while,
   but becomes a cached layer; recreations reuse it.

### Workspace bind
The [devcontainer.json](../.devcontainer/wsl/devcontainer.json) mounts the local workspace
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
missing — check the [devcontainer.json](../.devcontainer/wsl/devcontainer.json).
If no window opens: check `DISPLAY=:0` and the WSLg mounts.

---

## 4. Build (SCons)

| Command | Result |
|---|---|
| `scons help` | lists every command, variable and option (same as `scons -h`) |
| `scons` | builds the app for Linux **and** Windows (debug) |
| `scons target=linux` | Linux only → `build/linux/app` |
| `scons target=windows` | Windows only → `build/windows/app.exe` |
| `scons mode=release` | optimized (`-O2`) |
| `scons --helloworld` | builds **only** the helloworld test instead of the app |
| `scons -r` / `scons --run` | after the build, runs the binary for the current OS |
| `scons -c` | cleans the artifacts (app **and** helloworld) |

Options combine, e.g. `scons target=linux --helloworld -r`. An unknown command, option or
variable (or an invalid `target=`/`mode=` value) stops the build with an error that points
to `scons help`.

About `-r` / `--run`:
- Inside the container the OS is Linux, so it runs `build/linux/<app>`. With
  `target=all` (default) both are built, but only the Linux binary runs.
- With `target=windows` nothing runs: SCons prints a warning that a `.exe` can't run on
  Linux and suggests `target=linux`. If the container is on a Windows host (WSL2), the
  Linux build already opens its window on the Windows desktop. The `.exe` is meant for
  developing directly on Windows or for a final Windows build.
- SCons itself prints `Warning: the -r option is not yet implemented` because `-r` is a
  legacy option name it reserves. Ignore it, or use `--run` to avoid it.

- **Linux:** links GLFW installed in `/usr/local` + GLEW/GL via `pkg-config` (static).
- **Windows:** cross-compiled with `x86_64-w64-mingw32-g++`, GLFW from
  `/opt/glfw/build-windows`, GLEW compiled from source (`glew.c`, `GLEW_STATIC`).
  The `.exe` is **self-contained** (`-static`), with no DLL dependencies.

---

## 5. Run

### Linux (inside the container, window on Windows via WSLg)
```bash
scons target=linux -r                 # app
scons target=linux --helloworld -r    # helloworld test
```

### Windows (outside the container)
`build/windows/app.exe` (or `helloworld.exe` with `--helloworld`) lives in the workspace bind, so it shows up on Windows at
`...\The_Cherno_OpenGL_Introduction\build\windows\`. Open it in a **second window**
(Explorer or another VS Code window on Windows) and run it directly.

---

## 6. Use a custom GLFW fork
The container clones GLFW at creation time. To point it at your fork, edit the `build.args`
in [devcontainer.json](../.devcontainer/wsl/devcontainer.json) and rebuild the container:

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
