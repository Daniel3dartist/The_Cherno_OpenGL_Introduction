# =============================================================================
# Devcontainer de desenvolvimento OpenGL (GLFW + GLEW)
# Base: Debian 13 (trixie) | Usuario: root | Build multiplataforma (Linux + Windows)
# GPU: compartilhada do host via WSL (/dev/dxg) + Mesa d3d12 -> qualquer fabricante.
# =============================================================================
FROM debian:trixie-slim

ENV DEBIAN_FRONTEND=noninteractive

# -----------------------------------------------------------------------------
# Toolchain + dependencias
# -----------------------------------------------------------------------------
RUN apt-get update && apt-get install -y --no-install-recommends \
    # Build nativo (Linux) + ferramentas
    build-essential g++ pkg-config scons git cmake ca-certificates curl unzip \
    # GLEW + OpenGL (Linux). GLFW NAO vem do apt: e compilado do fonte abaixo.
    libglew-dev libgl1-mesa-dev \
    # Dependencias para compilar o GLFW do fonte (backends X11 + Wayland)
    libx11-dev libxrandr-dev libxinerama-dev libxcursor-dev libxi-dev \
    libxkbcommon-dev libwayland-dev wayland-protocols extra-cmake-modules \
    # Mesa (driver d3d12 p/ GPU acelerada via WSL) + utilitarios de diagnostico
    mesa-utils libgl1-mesa-dri libglx-mesa0 libegl-mesa0 mesa-vulkan-drivers \
    # Cross-compile para Windows (fornece x86_64-w64-mingw32-g++/gcc/windres/ranlib)
    g++-mingw-w64-x86-64 binutils-mingw-w64-x86-64 mingw-w64-tools \
 && rm -rf /var/lib/apt/lists/*

# -----------------------------------------------------------------------------
# GLFW: clonar e compilar do fonte para Linux e Windows (na criacao da imagem).
# URL/ref configuraveis via build.args (trocar pelo fork depois).
# -----------------------------------------------------------------------------
ARG GLFW_REPO=https://github.com/glfw/glfw.git
ARG GLFW_REF=master

RUN git clone --depth 1 --branch "${GLFW_REF}" "${GLFW_REPO}" /opt/glfw/src

# Build Linux (biblioteca estatica) + instalacao em /usr/local.
# A instalacao gera o glfw3.pc com as deps privadas (X11/Wayland/dl/pthread),
# entao o link estatico resolve via `pkg-config --static --libs glfw3`.
RUN cmake -S /opt/glfw/src -B /opt/glfw/build-linux \
      -DBUILD_SHARED_LIBS=OFF \
      -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF \
      -DGLFW_BUILD_X11=ON -DGLFW_BUILD_WAYLAND=ON \
 && cmake --build /opt/glfw/build-linux -j"$(nproc)" \
 && cmake --install /opt/glfw/build-linux

# Build Windows (cross, biblioteca estatica) usando o toolchain file do proprio GLFW
RUN cmake -S /opt/glfw/src -B /opt/glfw/build-windows \
      -DBUILD_SHARED_LIBS=OFF \
      -DCMAKE_TOOLCHAIN_FILE=/opt/glfw/src/CMake/x86_64-w64-mingw32.cmake \
      -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF \
 && cmake --build /opt/glfw/build-windows -j"$(nproc)"

# -----------------------------------------------------------------------------
# GLEW: fonte (ja traz glew.c gerado) para o alvo Windows.
# No Linux usamos o pacote libglew-dev; no Windows o SConstruct compila glew.c.
# -----------------------------------------------------------------------------
ARG GLEW_VERSION=2.2.0
RUN curl -fsSL -o /tmp/glew.tgz \
      "https://github.com/nigels-com/glew/releases/download/glew-${GLEW_VERSION}/glew-${GLEW_VERSION}.tgz" \
 && mkdir -p /opt/glew \
 && tar -xzf /tmp/glew.tgz -C /opt/glew --strip-components=1 \
 && rm -f /tmp/glew.tgz

# Sem instrucao USER: o usuario do container permanece root (default do Debian).
WORKDIR /workspace
