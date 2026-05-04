# 01 — Sistema de Estados do Jogo (FSM Global)

A máquina de estados que controla o fluxo do jogo do ponto de vista do usuário: do menu inicial à batalha tática, passando por save/load, eventos modais e telas de game over.

---

## 1. Estados

```cpp
UENUM(BlueprintType)
enum class EGameFlowState : uint8
{
    MainMenu,
    Loading,
    Running,    // mapa-mundi rodando, tempo passando
    Paused,     // mapa-mundi congelado (tempo = 0), UI ativa
    Battle,     // simulação macro pausada, batalha tática ativa
    Event,      // popup modal, tempo congelado, espera decisão
    GameOver
};
```

---

## 2. Onde Mora a FSM

`UGameFlowSubsystem : public UGameInstanceSubsystem`

Vive na `GameInstance` porque **persiste entre mapas** (MainMenu → Mapa Estratégico → Mapa de Batalha → volta).

---

## 3. Transições Permitidas

```
MainMenu  ──► Loading ──► Running
Running   ◄──► Paused
Running   ──► Event ──► Running
Running   ──► Loading (Battle map) ──► Battle ──► Loading ──► Running
Running   ──► GameOver
Qualquer  ──► MainMenu (via menu)
```

---

## 4. Contrato de Transição

```cpp
class IGameFlowListener
{
    virtual void OnEnterState(EGameFlowState NewState, EGameFlowState OldState) = 0;
    virtual void OnExitState (EGameFlowState OldState, EGameFlowState NewState) = 0;
};
```

Subsistemas se registram. O `TimeSubsystem` ouve para zerar o tick em `Paused/Event/Battle`. O `WorldSimSubsystem` ouve para suspender a economia. UI ouve para trocar de HUD.

---

## 5. Estado Global vs Estado Local

| Tipo | Onde vive | Exemplo | Persistência |
|---|---|---|---|
| **Global** | `GameInstance` + `GameInstanceSubsystem` | FSM, save atual, configurações, mundo simulado | Entre mapas |
| **Local** | `GameState` + `WorldSubsystem` | Câmera, seleção, HUD do mapa atual | Por mapa |

**Truque chave**: o **mundo simulado (províncias, nações, economia) vive na GameInstance**, não no GameState. Isso permite carregar o mapa de batalha sem destruir a simulação macro.

---

## 6. Fluxos Principais

### Iniciar jogo novo

```
MainMenu → Loading
  └─ NewGameService cria WorldState (nações, províncias, comandantes)
  └─ Carrega mapa estratégico
  └─ FSM → Running
  └─ TimeSubsystem.Start()
```

### Pausar

```
Running → Paused
  └─ TimeSubsystem.SetScale(0)
  └─ UI mostra overlay
  └─ Subsistemas continuam respondendo a input mas não tickam
```

### Entrar em batalha

```
Running → Event (proposta de batalha) → Loading → Battle
  └─ BattleService snapshota o estado militar relevante
  └─ Carrega BattleMap (level streaming)
  └─ BattleSubsystem instanciado
  └─ Resultado escrito de volta no WorldState
  └─ Loading → Running
```

### Carregar save

```
Qualquer → MainMenu → Loading
  └─ SaveSubsystem.Deserialize(WorldState)
  └─ FSM → Running no estado salvo
```

---

## 7. Pontos de Atenção

- A FSM é **autoridade**: nenhum subsistema deve assumir que está rodando. Sempre checar o estado atual ou se inscrever nas transições.
- Transições proibidas (ex: `Battle → MainMenu` direto sem passar por `Loading`) devem **falhar com log**, não silenciosamente.
- Estado `Event` é distinto de `Paused`: durante `Event` a UI é modal (popup obrigatório), durante `Paused` a UI é normal mas tempo está em zero.
- `Loading` deve ser explícito mesmo em transições rápidas para dar oportunidade de carregar/descarregar streaming levels e fazer fades.
