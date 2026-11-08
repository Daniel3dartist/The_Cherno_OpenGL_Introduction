# Setup & Compilação

Guia para montar o ambiente, compilar (Linux + Windows) e abrir a renderização OpenGL
de dentro do container no Windows para debugar a janela.

---

## 1. Pré-requisitos no host (Windows)

| Componente | Observação |
|---|---|
| Windows 11 | com **WSL2** habilitado (`wsl --update`) |
| Distro WSL2 | qualquer (ex. Ubuntu/Debian) — fornece o **WSLg** e `/dev/dxg` |
| Driver de GPU | driver do fabricante instalado **no Windows** (NVIDIA/AMD/Intel). É ele que o WSL compartilha |
| Podman | instalado **dentro do WSL2** |
| VS Code | com a extensão **Dev Containers** (`ms-vscode-remote.remote-containers`) |

### Fazer o VS Code usar o Podman
Nas *Settings* do VS Code (lado WSL / Remote), defina:

```jsonc
"dev.containers.dockerPath": "podman"
```

> O container roda como **root** (requisito do projeto). Em Podman *rootless*, o root do
> container é mapeado para o seu usuário do WSL — suficiente para acessar `/dev/dxg`
> (normalmente `0666`) e escrever no bind do workspace. Se o device não aparecer, rode
> o Podman *rootful* (`sudo podman ...`) ou ajuste as permissões de `/dev/dxg`.

---

## 2. Abrir no devcontainer

1. Clone o projeto no Windows (ex. `C:\Users\<voce>\Documents\projects\The_Cherno_OpenGL_Introduction`).
2. Abra a pasta no VS Code.
3. `F1` → **Dev Containers: Reopen in Container**.
4. A primeira criação **compila o GLFW do fonte** (Linux + Windows) — demora um pouco,
   mas vira camada cacheada; recriações reaproveitam.

### Bind do workspace
O [devcontainer.json](../.devcontainer/devcontainer.json) monta o caminho local do
Windows (via WSL2) em `/workspace`:

```jsonc
"workspaceMount": "source=${localWorkspaceFolder},target=/workspace,type=bind,consistency=cached",
"workspaceFolder": "/workspace"
```

## 3. GPU e renderização no Windows (como funciona)

Em Windows + WSL2 **não existe** GPU "sem virtualização nenhuma": o caminho real é a GPU
do host **compartilhada pela camada de compute do WSL** (GPU-PV) — **sem** passthrough
dedicado e **sem** software render. É o que o devcontainer configura:

- `--device=/dev/dxg` → acesso à GPU do host.
- mount `/usr/lib/wsl` + `LD_LIBRARY_PATH=/usr/lib/wsl/lib` → loader da GPU do WSL.
- **Mesa** com o driver **d3d12** → traduz OpenGL para a GPU via D3D12. Por isso funciona
  com **qualquer fabricante** (NVIDIA/AMD/Intel) e **qualquer GPU** (onboard ou offboard):
  a detecção é automática, não há nada fixo para a RTX 5060.
- mounts de `/mnt/wslg` + `/tmp/.X11-unix` e `DISPLAY=:0` → o **WSLg** exibe a janela
  direto no Windows (sem X server extra tipo VcXsrv).

### Validar a GPU e o display
Dentro do container:

```bash
glxinfo -B      # "OpenGL renderer string" deve mostrar sua GPU (ex. NVIDIA ...),
                # e NAO "llvmpipe" (que seria software render)
glxgears        # deve abrir uma janela com engrenagens no seu desktop Windows
```

Se aparecer `llvmpipe`: faltou `/dev/dxg`, o mount `/usr/lib/wsl` ou o `LD_LIBRARY_PATH`
— confira o [devcontainer.json](../.devcontainer/devcontainer.json).
Se nenhuma janela abrir: confira `DISPLAY=:0` e os mounts do WSLg.

---

## 4. Compilar (SCons)

| Comando | Resultado |
|---|---|
| `scons` | compila Linux **e** Windows (debug) |
| `scons target=linux` | só Linux → `build/linux/helloworld` |
| `scons target=windows` | só Windows → `build/windows/helloworld.exe` |
| `scons mode=release` | otimizado (`-O2`) |
| `scons -c` | limpa os artefatos |

- **Linux:** linka o GLFW instalado em `/usr/local` + GLEW/GL via `pkg-config` (estático).
- **Windows:** cross-compile com `x86_64-w64-mingw32-g++`, GLFW de
  `/opt/glfw/build-windows`, GLEW compilado do fonte (`glew.c`, `GLEW_STATIC`).
  O `.exe` é **self-contained** (`-static`), sem depender de DLLs.

---

## 5. Rodar

### Linux (dentro do container, janela no Windows via WSLg)
```bash
scons target=linux
./build/linux/helloworld
```

### Windows (fora do container)
O `build/windows/helloworld.exe` está no bind do workspace, então aparece no Windows em
`...\The_Cherno_OpenGL_Introduction\build\windows\`. Abra numa **segunda janela** (Explorer
ou outra janela do VS Code no Windows) e execute diretamente.

---

## 6. Usar um fork customizado do GLFW
O container clona o GLFW na criação. Para apontar para o seu fork, edite os `build.args`
em [devcontainer.json](../.devcontainer/devcontainer.json) e recrie o container:

```jsonc
"args": {
  "GLFW_REPO": "https://github.com/<voce>/glfw.git",
  "GLFW_REF": "minha-branch"   // ou uma tag, ex. "3.4"
}
```

---

## 7. Troubleshooting

| Sintoma | Causa provável / solução |
|---|---|
| `glxinfo` mostra `llvmpipe` | faltou `/dev/dxg`, `/usr/lib/wsl` ou `LD_LIBRARY_PATH` |
| Janela não abre | `DISPLAY` incorreto ou mounts do WSLg ausentes |
| `permission denied` em `/dev/dxg` | Podman rootless sem acesso — rode rootful ou ajuste permissões |
| Build lento / I/O lento | o workspace é bind 9p (`/mnt/c`); o build do GLFW fica em `/opt` (rápido) |
| `pkg-config: glfw3 not found` | a imagem não concluiu o `cmake --install` do GLFW — rebuild do container |
