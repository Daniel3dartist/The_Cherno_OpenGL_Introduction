# Hello World — setup and screen-stream test

The [`helloworld/`](../helloworld/) directory contains the OpenGL "hello world": a
**colored triangle**. It validates, end to end:

1. the **build** (native Linux and Windows cross-compile);
2. the **GPU acceleration** of the host inside the container;
3. the **screen stream** from the container to the Windows desktop (WSLg).

Files:
- [`helloworld/main.cpp`](../helloworld/main.cpp) — GLFW + GLEW, OpenGL 3.3 core, VAO/VBO, inline shaders.
- [`helloworld/SConscript`](../helloworld/SConscript) — build target (Linux + Windows). Only
  built when `--helloworld` is passed; a plain `scons` builds the app in `src/`.

---

## Step by step

### 1. Display + GPU test (Linux, inside the container)
```bash
scons target=linux --helloworld -r     # build + run (or: ./build/linux/helloworld)
```
**Expected:**
- An `OpenGL Hello Triangle` window opens **on your Windows desktop** (via WSLg) with a
  triangle whose vertices are red/green/blue on a dark background.
- In the terminal, the lines:
  ```
  GL_VENDOR   : ...
  GL_RENDERER : <your GPU>       <- should be your GPU, NOT "llvmpipe"
  GL_VERSION  : ...
  ```
- `ESC` closes the window.

If `GL_RENDERER` shows `llvmpipe` or the window does not open, see the Troubleshooting
section in [SETUP.md](SETUP.md).

### 2. Cross-compile test (Windows, outside the container)
```bash
scons target=windows --helloworld
```
This produces `build/windows/helloworld.exe`. Since the workspace is bind-mounted at the
Windows path, the file shows up at:

```
...\The_Cherno_OpenGL_Introduction\build\windows\helloworld.exe
```

Open a **second window** on Windows (Explorer or another VS Code window on the host) and
run the `.exe` directly. It should open the same triangle window, now as a **native
Windows** application (no container).

---

## What each test confirms

| Test | Confirms |
|---|---|
| `build/linux/helloworld` opens the window | devcontainer + GPU + WSLg + Linux GLFW/GLEW OK |
| `GL_RENDERER` = real GPU (not llvmpipe) | host GPU shared correctly |
| `build/windows/helloworld.exe` runs on Windows | MinGW cross-compile + Windows GLFW/GLEW OK |
| `.exe` visible in Windows Explorer | workspace bind OK |
