# The_Cherno_OpenGL_Introduction

Projeto para acompanhar o tutorial de **OpenGL** do [The Cherno](https://www.youtube.com/playlist?list=PLlrATfBNZ98foTJPJ_Ev03o2oq3-GGOS2),
usando **GLFW** + **GLEW**. O desenvolvimento acontece num **devcontainer** (Debian 13),
com build **multiplataforma** (Linux + Windows) e renderização OpenGL acelerada por GPU
exibida direto no Windows via WSLg.

---

## Ambiente de desenvolvimento

O desenvolvimento é feito dentro de um devcontainer. O container compila para Linux
(execução dentro do container, janela no Windows via WSLg) e faz cross-compile de um
`.exe` para Windows (execução nativa, fora do container).

### Componentes e versões

| Componente | Versão / detalhe |
|---|---|
| SO do container | Debian **13 (trixie)** |
| Usuário do container | **root** |
| OpenGL | **3.3 core** |
| GLFW | **compilado do fonte** no container (upstream `master` ≈ 3.6-dev; configurável via `GLFW_REF`) |
| GLEW | **2.2.0** (Linux via `libglew-dev`; Windows compilado do fonte) |
| Compilador Linux | `g++` (trixie) |
| Cross-compile Windows | **MinGW-w64** (`x86_64-w64-mingw32-g++`) |
| Build system | **SCons** |
| GPU / OpenGL driver | **Mesa** (driver `d3d12`, via WSL `/dev/dxg`) |

### Dependências do host (Windows)

- **Windows 11** com **WSL2** (`wsl --update`) e driver de GPU instalado no Windows
  (NVIDIA / AMD / Intel — a detecção é automática, onboard ou offboard).
- **Podman** instalado dentro do WSL2.
- **VS Code** + extensão **Dev Containers** (defina `"dev.containers.dockerPath": "podman"`).

### Compilar (resumo)

```bash
scons                    # Linux + Windows (debug)
scons target=linux       # -> build/linux/helloworld
scons target=windows     # -> build/windows/helloworld.exe
scons mode=release       # otimizado
```

### Documentação

- **[.doc/SETUP.md](.doc/SETUP.md)** — pré-requisitos, como abrir no devcontainer, como a
  GPU/renderização chega ao Windows, como compilar e como debugar a janela.
- **[.doc/HELLOWORLD.md](.doc/HELLOWORLD.md)** — triângulo de teste ([`helloworld/`](helloworld/))
  para validar o setup, a GPU e o stream de tela.

---

## Licença

Ver [LICENSE](LICENSE).
