# =============================================================================
# OpenGL (GLFW + GLEW) development devcontainer.
# Base: Debian 13 (trixie) | User: root | Cross-platform build (Linux + Windows)
# GPU: host GPU shared via WSL (/dev/dxg) + Mesa d3d12 -> any vendor.
# =============================================================================
FROM debian:trixie-slim

ENV DEBIAN_FRONTEND=noninteractive

# -----------------------------------------------------------------------------
# Toolchain + dependencies
# -----------------------------------------------------------------------------
RUN apt-get update && apt-get install -y --no-install-recommends \
    # Native (Linux) build + tooling
    build-essential g++ pkg-config scons git cmake ca-certificates curl unzip \
    # GLEW + OpenGL (Linux). GLFW is NOT from apt: it is built from source below.
    libglew-dev libgl1-mesa-dev \
    # Dependencies to build GLFW from source (X11 + Wayland backends)
    libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
    libxkbcommon-dev libwayland-dev wayland-protocols extra-cmake-modules \
    # Mesa (d3d12 driver for GPU acceleration via WSL) + diagnostic utilities
    mesa-utils libgl1-mesa-dri libglx-mesa0 libegl-mesa0 mesa-vulkan-drivers \
    # Windows cross-compile (provides x86_64-w64-mingw32-g++/gcc/windres/ranlib)
    g++-mingw-w64-x86-64 binutils-mingw-w64-x86-64 mingw-w64-tools \
 && rm -rf /var/lib/apt/lists/*

# -----------------------------------------------------------------------------
# GLFW: clone and build from source for Linux and Windows (at image build time).
# URL/ref configurable via build.args (swap for a fork later).
# -----------------------------------------------------------------------------
ARG GLFW_REPO=https://github.com/glfw/glfw.git
ARG GLFW_REF=master

RUN git clone --depth 1 --branch "${GLFW_REF}" "${GLFW_REPO}" /opt/glfw/src

# Linux build (static library) + install into /usr/local.
# Installing generates glfw3.pc with the private deps (X11/Wayland/dl/pthread),
# so static linking resolves them via `pkg-config --static --libs glfw3`.
RUN cmake -S /opt/glfw/src -B /opt/glfw/build-linux \
      -DBUILD_SHARED_LIBS=OFF \
      -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF \
      -DGLFW_BUILD_X11=ON -DGLFW_BUILD_WAYLAND=ON \
 && cmake --build /opt/glfw/build-linux -j"$(nproc)" \
 && cmake --install /opt/glfw/build-linux

# Windows build (cross, static library) using GLFW's own toolchain file.
RUN cmake -S /opt/glfw/src -B /opt/glfw/build-windows \
      -DBUILD_SHARED_LIBS=OFF \
      -DCMAKE_TOOLCHAIN_FILE=/opt/glfw/src/CMake/x86_64-w64-mingw32.cmake \
      -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF \
 && cmake --build /opt/glfw/build-windows -j"$(nproc)"

# -----------------------------------------------------------------------------
# GLEW: source (ships a pre-generated glew.c) for the Windows target.
# On Linux we use the libglew-dev package; on Windows SConstruct compiles glew.c.
# -----------------------------------------------------------------------------
ARG GLEW_VERSION=2.2.0
RUN curl -fsSL -o /tmp/glew.tgz \
      "https://github.com/nigels-com/glew/releases/download/glew-${GLEW_VERSION}/glew-${GLEW_VERSION}.tgz" \
 && mkdir -p /opt/glew \
 && tar -xzf /tmp/glew.tgz -C /opt/glew --strip-components=1 \
 && rm -f /tmp/glew.tgz

# No USER instruction: the container user stays root (Debian default).
WORKDIR /workspace
