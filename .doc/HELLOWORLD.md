# Hello World — teste do setup e do stream de tela

O diretório [`helloworld/`](../helloworld/) contém o "hello world" do OpenGL: um
**triângulo colorido**. Ele serve para validar, de ponta a ponta:

1. o **build** (Linux nativo e Windows cross-compile);
2. a **GPU acelerada** do host dentro do container;
3. o **stream de tela** do container para o desktop do Windows (WSLg).

Arquivos:
- [`helloworld/main.cpp`](../helloworld/main.cpp) — GLFW + GLEW, OpenGL 3.3 core, VAO/VBO, shaders inline.
- [`helloworld/SConscript`](../helloworld/SConscript) — alvo de build (Linux + Windows).

---

## Passo a passo

### 1. Teste do display + GPU (Linux, dentro do container)
```bash
scons target=linux
./build/linux/helloworld
```
**Esperado:**
- Uma janela `OpenGL Hello Triangle` abre **no seu desktop Windows** (via WSLg) com um
  triângulo de vértices vermelho/verde/azul sobre fundo escuro.
- No terminal, as linhas:
  ```
  GL_VENDOR   : ...
  GL_RENDERER : <sua GPU>        <- deve ser sua GPU, NAO "llvmpipe"
  GL_VERSION  : ...
  ```
- `ESC` fecha a janela.

Se o `GL_RENDERER` mostrar `llvmpipe` ou a janela não abrir, veja a seção de
Troubleshooting em [SETUP.md](SETUP.md).

### 2. Teste do cross-compile (Windows, fora do container)
```bash
scons target=windows
```
Isso gera `build/windows/helloworld.exe`. Como o workspace é bind-montado no caminho do
Windows, o arquivo aparece em:

```
...\The_Cherno_OpenGL_Introduction\build\windows\helloworld.exe
```

Abra uma **segunda janela** no Windows (Explorer ou outra janela do VS Code no host) e
execute o `.exe` diretamente. Deve abrir a mesma janela do triângulo, agora como
aplicação **nativa do Windows** (sem container).

---

## O que cada teste confirma

| Teste | Confirma |
|---|---|
| `build/linux/helloworld` abre a janela | devcontainer + GPU + WSLg + GLFW/GLEW Linux OK |
| `GL_RENDERER` = GPU real (não llvmpipe) | GPU do host compartilhada corretamente |
| `build/windows/helloworld.exe` roda no Windows | cross-compile MinGW + GLFW/GLEW Windows OK |
| `.exe` visível no Explorer do Windows | bind do workspace OK |
