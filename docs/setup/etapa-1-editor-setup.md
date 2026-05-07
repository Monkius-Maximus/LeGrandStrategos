# Setup do Editor — Etapa 1 (MVP)

Este documento lista o trabalho que **não é feito por C++** e precisa ser executado dentro do Editor da Unreal para que o esqueleto programado vire um jogo rodável.

A ordem aqui é a recomendada — cada passo depende dos anteriores.

---

## 0. Pré-requisitos

- Unreal Engine **5.5** instalado (associação configurada no `.uproject`).
- Visual Studio 2022 (Windows) ou Rider/CLion (Linux/Mac) com toolchain C++.
- Plugin `EnhancedInput` habilitado (já está em `LeGrandStrategos.uproject`).

Antes de abrir o Editor, gere os arquivos de projeto:

- **Windows**: clique direito em `LeGrandStrategos.uproject` → *Generate Visual Studio project files*
- **Linux/Mac**: rode `./GenerateProjectFiles.sh` da raiz da UE

Build em modo **Development Editor** e abra o editor pelo binário gerado.

---

## 1. Criar a pasta de assets

No *Content Browser* crie a estrutura:

```
Content/
  Bootstrap/        ← DataTables e UWorldBootstrapAsset
  Input/            ← Input Actions e Mapping Contexts
  Materials/        ← M_Province, M_Army
  Maps/             ← L_StrategySandbox.umap
  UI/
    HUD/            ← WBP_HUD
    MainMenu/       ← WBP_MainMenu
  Visuals/
    Province/       ← BP_ProvinceVisual
    Army/           ← BP_ArmyVisual
    Map/            ← BP_MapActor
    Camera/         ← BP_CameraPawn
    Player/         ← BP_PlayerController
```

---

## 2. Input (Enhanced Input)

### 2.1 Input Actions

Em `Content/Input/`:

| Asset | Tipo | Notas |
|---|---|---|
| `IA_MoveCamera` | `Input Action`, Value Type **Axis2D (Vector2D)** | WASD pan |
| `IA_ZoomCamera` | `Input Action`, Value Type **Axis1D (float)** | scroll |
| `IA_SelectClick` | `Input Action`, Value Type **Digital (bool)** | LMB |
| `IA_OrderMove` | `Input Action`, Value Type **Digital (bool)** | RMB |

### 2.2 Mapping Contexts

`IMC_Camera`:
- `IA_MoveCamera` ← W (Y +), S (Y -), A (X -), D (X +) com modifier *Negate* nos eixos negativos ou via *Swizzle*.
- `IA_ZoomCamera` ← `Mouse Wheel Axis`.

`IMC_Selection`:
- `IA_SelectClick` ← `Left Mouse Button`.
- `IA_OrderMove` ← `Right Mouse Button`.

(Você pode usar **um** Mapping Context único também — só lembre que o Pawn registra prioridade 0 e o PlayerController prioridade 1.)

---

## 3. Materiais

### 3.1 `M_Province`

- Domain: **Surface**, Shading Model: **Unlit** (placeholder; troque depois).
- Vector Parameter `BaseColor` (RGB). Conecte direto em *Emissive Color*.
- (Opcional) Scalar Parameter `OutlineIntensity` para futuro destaque.

### 3.2 `M_Army`

- Igual a `M_Province` mas com Texture Parameter `BaseTexture` somado/multiplicado em `BaseColor`. Aceite null (use `if Texture is None → BaseColor`).

---

## 4. Blueprints derivados das classes C++

Para cada uma destas, no Content Browser → *Add → Blueprint Class → All Classes*, escolha o pai e nomeie:

| BP | Pai C++ | Pasta |
|---|---|---|
| `BP_StrategosGameMode` | `AStrategosGameMode` | `Content/` |
| `BP_StrategosCameraPawn` | `AStrategosCameraPawn` | `Visuals/Camera/` |
| `BP_StrategosPlayerController` | `AStrategosPlayerController` | `Visuals/Player/` |
| `BP_StrategosMapActor` | `AStrategosMapActor` | `Visuals/Map/` |
| `BP_ProvinceVisual` | `AStrategosProvinceVisualActor` | `Visuals/Province/` |
| `BP_ArmyVisual` | `AStrategosArmyVisualActor` | `Visuals/Army/` |
| `WBP_HUD` | `UStrategosHUDWidget` | `UI/HUD/` |
| `WBP_MainMenu` | `UStrategosMainMenuWidget` | `UI/MainMenu/` |

### 4.1 `BP_StrategosCameraPawn`

- Defaults → *Strategos|Camera|Input*:
  - `DefaultMappingContext` = `IMC_Camera`
  - `MoveAction` = `IA_MoveCamera`
  - `ZoomAction` = `IA_ZoomCamera`
- Tuning opcional: `PanSpeed`, `MinZoomDistance`, etc.

### 4.2 `BP_StrategosPlayerController`

- Defaults → *Strategos|Input*:
  - `SelectionMappingContext` = `IMC_Selection`
  - `SelectClickAction` = `IA_SelectClick`
  - `OrderMoveAction` = `IA_OrderMove`

### 4.3 `BP_ProvinceVisual`

- *Mesh* = `/Engine/BasicShapes/Plane` (escala ~8×8×1 cm).
- Defaults:
  - `ProvinceMaterial` = `M_Province`
  - `WorldUnitsPerMapCell` = 1000
- Confirme que *Static Mesh* tem `Collision Complexity = Simple`, `Body Instance > Block on Visibility = true` (necessário para hover/click do Actor).

### 4.4 `BP_ArmyVisual`

- *Mesh* = `/Engine/BasicShapes/Cube` (escala ~1×1×1 cm) ou um sprite plane.
- Defaults:
  - `ArmyMaterial` = `M_Army`
  - `PlaceholderTexture` = (vazio agora; aponte qualquer ícone para teste)

### 4.5 `BP_StrategosMapActor`

- Defaults:
  - `ProvinceVisualClass` = `BP_ProvinceVisual`
  - `ArmyVisualClass` = `BP_ArmyVisual`
  - `SpawnDelaySeconds` = 0.1

### 4.6 `BP_StrategosGameMode`

- Defaults:
  - `Default Pawn Class` = `BP_StrategosCameraPawn`
  - `Player Controller Class` = `BP_StrategosPlayerController`
  - `HUD Class` = (deixar default ou criar AHUD child se quiser)
  - `Bootstrap Asset` = (vazio para usar `ApplyDefaultSandbox`; aponte um `UWorldBootstrapAsset` quando tiver as DataTables)

### 4.7 `WBP_HUD`

- Layout sugestão: top bar com data + botões `Pause / Slow / Normal / Fast / Fastest`, e painel lateral para província selecionada.
- Use `Bind` em TextBlocks: ligue para `GetCurrentDateText`, `GetSelectedProvinceName`, `GetSelectedProvinceOwnerName`.
- OnClick dos botões → chamar `PauseGame` / `SetTimeSpeed(ETimeSpeed::Slow)` etc.
- Em `BeginPlay` do Player Controller (ou GameMode), faça `Create Widget` (`WBP_HUD`) → `Add to Viewport`.

### 4.8 `WBP_MainMenu`

- Botões `New Game`, `Load Game`, `Quit`.
- New Game → `NewGame("L_StrategySandbox")`.
- Quit → `QuitGame()`.

---

## 5. Level

1. *File → New Level → Empty Level* → salve como `Content/Maps/L_StrategySandbox.umap`.
2. Adicione um `BP_StrategosMapActor` na origem (0,0,0).
3. Adicione um `Directional Light` para iluminação básica.
4. Adicione um `Sky Atmosphere` se quiser fundo bonito (opcional).

Em *Project Settings → Maps & Modes*:
- `Editor Startup Map` = `L_StrategySandbox`
- `Game Default Map` = `L_StrategySandbox`
- `Default GameMode` = `BP_StrategosGameMode`

---

## 6. (Opcional) DataTables para conteúdo via assets

Em `Content/Bootstrap/`:

1. *Add → Miscellaneous → Data Table*. Para cada uma escolha a *Row Structure*:
   - `DT_Provinces` → `ProvinceRow`
   - `DT_Nations` → `NationRow`
   - `DT_Armies` → `ArmyRow`
   - `DT_NationalIdeas` → `NationalIdeaRow`

2. Preencha 3-4 linhas por tabela (use o sandbox programático em `WorldBootstrapper.cpp` como referência de schema).

3. *Add → Miscellaneous → Data Asset → UWorldBootstrapAsset*: nomeie `DA_Sandbox1836`. Aponte as 4 tables e `PlayerNationId = "Albion"`.

4. Em `BP_StrategosGameMode`, defina `Bootstrap Asset = DA_Sandbox1836`.

Se algum passo aqui falhar, o `RunBootstrap` cai no `ApplyDefaultSandbox` automaticamente — você não fica com um mundo vazio.

---

## 7. Smoke test

Pressione **Play** no editor:

1. Logs em `Output Log` devem mostrar:
   - `StrategosGameMode BeginPlay.`
   - `Default sandbox applied: 3 nations, 11 provinces, 3 armies` (ou números do seu DataAsset).
   - `MapActor: spawned 11 province visuals, 3 army visuals`.
2. Você vê um grid 4×3 de quads coloridos: azul (Albion), vermelho (Galia), amarelo (Norden).
3. Clique numa província — outline brighter; HUD mostra nome + dono.
4. Pressione `Normal` na HUD — data avança.
5. Clique numa província sua, depois RMB numa adjacente — exército começa a se mover; após N dias o `BP_ArmyVisual` reposiciona.
6. Em alguns meses, exércitos da Galia/Norden empurram para fronteiras de Albion (comportamento Militarist do AIPlaceholderSubsystem).

Se faltar algo, confira *Output Log* — todos os subsistemas e o GameMode logam categoria própria (`LogStrategosCore`, `LogStrategosUI`, `LogStrategosAI`).

---

## 8. Onde editar depois

- **Cores das nações** — edite o sandbox em `Source/StrategosCore/Private/Bootstrap/WorldBootstrapper.cpp` ou via DataTable.
- **Velocidades de jogo** — `DaysPerSecondForSpeed` em `TimeSubsystem.cpp`.
- **Comportamento da IA** — `AIPlaceholderSubsystem.cpp`, métodos `Behavior_*`.
- **Custos de movimento** — `GetMovementCostDays` em `MilitarySubsystem.cpp`.

Mudanças em C++ requerem rebuild (Ctrl+Shift+B no VS / Hot Reload no editor).
