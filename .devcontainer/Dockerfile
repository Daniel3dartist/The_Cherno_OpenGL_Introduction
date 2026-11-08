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
    build-essential g++ gdb pkg-config scons git cmake ca-certificates curl unzip \
    # OpenGL headers/loader (Linux). GLFW and GLEW are NOT from apt: both are built
    # from source below as static libraries.
    libgl1-mesa-dev \
    # Dependencies to build GLFW from source (X11 + Wayland backends)
    libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
    libxkbcommon-dev libwayland-dev wayland-protocols extra-cmake-modules \
    # Mesa (d3d12 driver for GPU acceleration via WSL) + diagnostic utilities
    mesa-utils libgl1-mesa-dri libglx-mesa0 libegl-mesa0 mesa-vulkan-drivers \
    # Windows cross-compile (provides x86_64-w64-mingw32-g++/gcc/windres/ranlib)
    g++-mingw-w64-x86-64 binutils-mingw-w64-x86-64 mingw-w64-tools \
 && rm -rf /var/lib/apt/lists/*

# -----------------------------------------------------------------------------
# Static dependencies are staged in ${DEPS_STAGE} inside the image:
#   ${DEPS_STAGE}/linux/{include,lib}     -> libglfw3.a, libGLEW.a   (ELF x86_64)
#   ${DEPS_STAGE}/windows/{include,lib}   -> libglfw3.a, libglew32.a (MinGW PE x86_64)
# Both use the GNU ".a" extension (GCC/MinGW convention); the folder tells them apart.
# /workspace is a bind mount (only exists at container runtime and hides anything
# written there at image build time), so postCreateCommand in devcontainer.json
# copies ${DEPS_STAGE} into /workspace/dependencies.
# -----------------------------------------------------------------------------
ARG DEPS_STAGE=/opt/dependencies
ENV DEPS_STAGE=${DEPS_STAGE}

# -----------------------------------------------------------------------------
# GLFW: clone and build from source for Linux and Windows (at image build time).
# URL/ref configurable via build.args (swap for a fork later).
# -----------------------------------------------------------------------------
ARG GLFW_REPO=https://github.com/glfw/glfw.git
ARG GLFW_REF=master

RUN git clone --depth 1 --branch "${GLFW_REF}" "${GLFW_REPO}" /opt/glfw/src

# Linux build (static library) installed into ${DEPS_STAGE}/linux.
RUN cmake -S /opt/glfw/src -B /opt/glfw/build-linux \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="${DEPS_STAGE}/linux" \
      -DBUILD_SHARED_LIBS=OFF \
      -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF \
      -DGLFW_BUILD_X11=ON -DGLFW_BUILD_WAYLAND=ON \
 && cmake --build /opt/glfw/build-linux -j"$(nproc)" \
 && cmake --install /opt/glfw/build-linux

# Windows build (cross, static library) installed into ${DEPS_STAGE}/windows.
RUN cmake -S /opt/glfw/src -B /opt/glfw/build-windows \
      -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX="${DEPS_STAGE}/windows" \
      -DCMAKE_TOOLCHAIN_FILE=/opt/glfw/src/CMake/x86_64-w64-mingw32.cmake \
      -DBUILD_SHARED_LIBS=OFF \
      -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF \
 && cmake --build /opt/glfw/build-windows -j"$(nproc)" \
 && cmake --install /opt/glfw/build-windows

# -----------------------------------------------------------------------------
# GLEW: source release (ships a pre-generated glew.c), compiled as a static
# library for both targets with GLEW_STATIC (no DLL / .so).
# -----------------------------------------------------------------------------
ARG GLEW_VERSION=2.2.0
RUN curl -fsSL -o /tmp/glew.tgz \
      "https://github.com/nigels-com/glew/releases/download/glew-${GLEW_VERSION}/glew-${GLEW_VERSION}.tgz" \
 && mkdir -p /opt/glew \
 && tar -xzf /tmp/glew.tgz -C /opt/glew --strip-components=1 \
 && rm -f /tmp/glew.tgz

RUN set -e; \
    for p in linux windows; do \
      mkdir -p "${DEPS_STAGE}/$p/include/GL" "${DEPS_STAGE}/$p/lib" /opt/glew/build-$p; \
      cp /opt/glew/include/GL/*.h "${DEPS_STAGE}/$p/include/GL/"; \
    done; \
    # Linux: libGLEW.a
    gcc -O2 -fPIC -DGLEW_STATIC -DGLEW_NO_GLU -I/opt/glew/include \
        -c /opt/glew/src/glew.c -o /opt/glew/build-linux/glew.o; \
    ar rcs "${DEPS_STAGE}/linux/lib/libGLEW.a" /opt/glew/build-linux/glew.o; \
    # Windows: libglew32.a
    x86_64-w64-mingw32-gcc -O2 -DGLEW_STATIC -DGLEW_NO_GLU -I/opt/glew/include \
        -c /opt/glew/src/glew.c -o /opt/glew/build-windows/glew.o; \
    x86_64-w64-mingw32-ar rcs "${DEPS_STAGE}/windows/lib/libglew32.a" /opt/glew/build-windows/glew.o; \
    for p in linux windows; do \
      cp /opt/glew/LICENSE.txt "${DEPS_STAGE}/$p/GLEW-LICENSE.txt"; \
      cp /opt/glfw/src/LICENSE.md "${DEPS_STAGE}/$p/GLFW-LICENSE.md"; \
    done; \
    # Only static archives + headers are needed by the project (drop cmake/pkgconfig).
    rm -rf "${DEPS_STAGE:?}"/*/lib/cmake "${DEPS_STAGE:?}"/*/lib/pkgconfig

# No USER instruction: the container user stays root (Debian default).
WORKDIR /workspace
