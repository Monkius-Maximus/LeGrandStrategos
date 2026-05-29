# Onboarding UE5 + Workflow de Colaboração com o Claude

> Guia de primeiro contato com a Unreal Engine 5.5 para o Le Grand Strategos,
> e como trabalhar comigo (Claude Code) de forma eficiente conforme o projeto
> cresce. Leitura única; volte como referência.

---

## Parte 1 — Setup da Unreal Engine (primeiro contato)

O repositório é um projeto C++ de UE 5.5. **Binários não são versionados**
(`Binaries/`, `Intermediate/`, `Saved/` estão no `.gitignore`), então na
primeira vez a Engine precisa **compilar** o projeto a partir do código.

### 1.1 Pré-requisitos (Windows — caminho mais comum)

1. **Epic Games Launcher** → aba *Unreal Engine* → instalar **5.5.x**.
   - Em *Options* da instalação, marque **"Editor symbols for debugging"**
     (ajuda quando algo crasha) — opcional, ocupa espaço.
2. **Visual Studio 2022** (Community serve). No instalador, marque o workload
   **"Desenvolvimento de jogos com C++"**. Dentro dele, confirme:
   - *MSVC v143* (compilador C++ x64)
   - *Windows 10/11 SDK*
   - *.NET desktop development* (o UnrealBuildTool é C#)
   - Componente *IDE support for Unreal Engine* (instala integração)
3. Reinicie a máquina depois (associações de arquivo do `.uproject`).

> **Linux/Mac:** UE 5.5 roda, mas o fluxo difere (precisa compilar a Engine do
> source no Linux, ou usar a build do Launcher no Mac com Xcode). Se for o seu
> caso, me avise que adapto este guia. O resto do documento assume Windows.

### 1.2 Obter o projeto

Você já tem o repo. Garanta que está na branch certa e atualizado:

```
git fetch origin
git checkout claude/document-implementation-QLCWU
git pull origin claude/document-implementation-QLCWU
```

### 1.3 Gerar os arquivos de projeto

Clique com o botão direito em **`LeGrandStrategos.uproject`** →
**"Generate Visual Studio project files"**. Isso cria o `.sln` (que é
gitignored, e tudo bem — é derivado).

> Se a opção não aparecer no menu: abra o Epic Launcher → engrenagem ao lado do
> "Launch" da 5.5 → "Verify", e confirme que a 5.5 está marcada como a versão
> padrão. A associação do `.uproject` vem daí.

### 1.4 Primeira compilação

Duas formas — escolha uma:

**A) Pelo Visual Studio (recomendado na 1ª vez, mostra erros melhor):**
1. Abra `LeGrandStrategos.sln`.
2. Topo: configuração **`Development Editor`** + plataforma **`Win64`**.
3. *Build → Build Solution* (Ctrl+Shift+B).
4. Terminou sem erro → *Debug → Start Without Debugging* (Ctrl+F5) abre o Editor.

**B) Direto pelo `.uproject`:**
1. Duplo-clique no `.uproject`.
2. A Engine detecta que os módulos precisam compilar e pergunta
   *"... missing or built with a different engine version. Would you like to
   rebuild them now?"* → **Yes**.
3. Espera a compilação (a 1ª é lenta, minutos).

### 1.5 Confirmar que o módulo StrategosWorldGen carregou

Com o Editor aberto: **Window → Output Log**. Procure por:

```
LogStrategosWorldGen: StrategosWorldGen module started.
```

Se aparecer, o novo módulo está vivo. Se a compilação falhar, veja a
Parte 3 (como me reportar erros).

### 1.6 Iterar em código depois (Live Coding)

Quando você (ou eu) mudar C++:
- Com o Editor aberto, **Ctrl+Alt+F11** dispara **Live Coding** (recompila só
  o que mudou, em segundos). Bom para mudanças pequenas.
- Mudanças estruturais (novo `UCLASS`, novo `UPROPERTY`, novo módulo) às vezes
  exigem **fechar o Editor e rebuildar pelo VS**. Se o Live Coding falhar com
  erros estranhos, faça o rebuild completo.
- **Sempre que adicionar/remover arquivos `.h`/`.cpp`**, rode de novo o
  "Generate Visual Studio project files" (passo 1.3).

---

## Parte 2 — Como testar o worldgen no Editor

O `UWorldGenSubsystem` é um *GameInstanceSubsystem* com funções
`BlueprintCallable`. O caminho mais simples para ver um mapa gerado:

### 2.1 Via Level Blueprint (mais rápido para um teste)

1. Abra um nível qualquer → toolbar **Blueprints → Open Level Blueprint**.
2. Monte este grafo no **Event BeginPlay**:
   - `Get Game Instance` → `Get Subsystem` (classe = **WorldGenSubsystem**).
   - A partir do subsystem: **Generate World** (deixe os params no default, ou
     ajuste Seed/MapSize/Template).
   - Saída booleana → **Branch**. No *True*:
     - **Render Debug Texture** (Mode = `Biomes`) → guarda o `Texture2D` numa
       variável.
3. Para **ver** a textura na tela, crie um Widget UMG:
   - *Content Browser* → Add → *User Interface → Widget Blueprint*.
   - Adicione um **Image** que ocupe a tela.
   - No nível: `Create Widget` → `Add to Viewport`, e no widget faça
     `Set Brush from Texture` na Image usando a textura retornada.
4. **Play (PIE)**. O `GameInstance` só existe em PIE/runtime, então tem que dar
   Play — não aparece no preview do editor.

### 2.2 Validar estágio a estágio

Troque o parâmetro **Mode** do `Render Debug Texture` para inspecionar cada
etapa, na ordem da Seção 16 do documento de referência:

`RandomCells` → `Height` → `Coast` → `Temperature` → `Precipitation` → `Biomes`

Cada modo valida um estágio do pipeline visualmente.

### 2.3 Atalho opcional (posso implementar se você quiser)

Esse setup de UMG é trabalhoso para um primeiro contato. Posso criar um
`AWorldGenDebugActor` em C++ que, no `BeginPlay`, gera o mundo e desenha a
textura direto num plano/HUD — você só arrasta o actor pro nível e dá Play.
**Me peça** que adiciono numa próxima sessão.

---

## Parte 3 — Trabalhando comigo conforme o projeto cresce

A sua preocupação é legítima, mas o gargalo **não é o tamanho do repositório**
e sim **o que entra na conversa**. Eu não carrego o projeto inteiro: leio
arquivos sob demanda com ferramentas. Um repo de muitos GB de assets não me
atrapalha — *desde que* a gente seja específico sobre o que olhar.

### 3.1 O que eu CONSIGO e o que NÃO consigo ler

| Consigo ler bem | Não consigo (ou é inútil) |
|-----------------|---------------------------|
| Código C++ (`.h`/`.cpp`), `.Build.cs`, `.cs` | `.uasset` / `.umap` (binário opaco) |
| `.uproject`, `.ini`, `.json`, `.md` | Conteúdo de Blueprints como binário |
| **Texto de Blueprint copiado** (ver 3.3) | Texturas/áudio/meshes (binário) |
| **Screenshots** que você anexar | O que está só na sua tela do Editor |

> Regra de ouro: **se é binário do UE, eu não leio**. Traga a informação em
> texto ou imagem.

### 3.2 Mantenha o repo "magro" para o git (não para mim)

- O `.gitignore` já exclui `Binaries/`, `Intermediate/`, `Saved/`,
  `DerivedDataCache/` — ótimo, isso é lixo derivado.
- **Assets** (`.uasset`, `.umap`) **vão** ser versionados e são binários
  grandes. Para não inchar o git, configure **Git LFS** antes de começar a
  criar assets pesados:
  ```
  git lfs install
  git lfs track "*.uasset" "*.umap"
  git add .gitattributes
  ```
  (Me avise se quiser que eu prepare o `.gitattributes` e ajuste o `.gitignore`.)

### 3.3 O truque mais útil: **Blueprint como texto**

No Editor, dentro de um grafo de Blueprint, você pode **selecionar nós →
Ctrl+C → colar aqui na conversa**. O que vai pra área de transferência é
**texto legível** que eu entendo e consigo analisar/depurar. Vale também para:
- Copiar nós de um grafo para eu revisar a lógica.
- Copiar a config de um actor selecionado no nível.

Isso transforma "Blueprint opaco" em algo que dá pra colaborar de verdade.

### 3.4 Screenshots para resultado visual

Eu não vejo o que está no seu Editor. Para depurar o **resultado** do worldgen
(o mapa gerado), **anexe um screenshot** na conversa — eu leio imagens. Ex.:
"gerei com seed 1337, ficou assim [print]" e eu comparo com o esperado.

### 3.5 Reportar erros de compilação do jeito certo

Em vez de "não compilou", **cole o texto do erro**. No Visual Studio: janela
*Error List* ou *Output*. No Editor: *Output Log*. Copie as linhas que começam
com `error:` (e algumas acima/abaixo para contexto). Com o texto exato eu acho
a causa rápido; sem ele, fico adivinhando.

### 3.6 Ritmo de sessão (o que já está na CLAUDE.md)

- **Comece toda sessão colando o `PROJECT_STATE.md`** (o snapshot "onde
  paramos"). É o mecanismo principal de contexto — ver Parte 4.
- **1 sessão = 1 tarefa lógica = 1 commit.** Tarefas focadas economizam contexto
  e ficam mais fáceis de revisar.
- **Me aponte caminhos específicos** ("olha em `Source/StrategosWorldGen/
  Private/Pipeline/`") em vez de "olha o projeto". É mais rápido e barato.
- Se a conversa ficar muito longa, eu **resumo automaticamente** o contexto e
  continuo — você não precisa se preocupar em "encerrar antes".

### 3.7 O que versionar e trazer para o git

Versione: **código, `Config/*.ini`, `.uproject`, assets de jogo**.
Não versione: nada de `Binaries/Intermediate/Saved` (já ignorado).
Os arquivos de projeto da IDE (`.sln`, `.vcxproj`) são derivados — ignorados de
propósito; cada um regenera os seus.

---

## Parte 4 — Fluxo recomendado de uma sessão típica

1. Você abre a conversa e **cola o `PROJECT_STATE.md`** atualizado.
2. Descreve a tarefa do dia (foco único).
3. Eu leio só os arquivos relevantes, implemento, e explico em 1 frase por marco.
4. Você compila no UE (Live Coding ou rebuild) e, se der erro, **cola o texto**.
5. Itera até verde. Se for visual, você **anexa screenshot**.
6. Ao final, se pedir, eu **regenero o `PROJECT_STATE.md`** para você colar no
   Obsidian.
7. Commit/push acontece quando você autoriza (deploys são seus).

---

## Apêndice — Checklist de primeiro dia

- [ ] UE 5.5 instalada via Epic Launcher
- [ ] Visual Studio 2022 com workload C++ de jogos
- [ ] `Generate Visual Studio project files` no `.uproject`
- [ ] Build `Development Editor` / `Win64` sem erros
- [ ] Editor abre e Output Log mostra `StrategosWorldGen module started.`
- [ ] (Opcional) `git lfs install` + track de `.uasset`/`.umap`
- [ ] Teste do worldgen via Level Blueprint em PIE
- [ ] Anexar screenshot do mapa gerado na próxima conversa
