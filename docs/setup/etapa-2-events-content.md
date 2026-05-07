# Setup do Editor — Etapa 2 Events

Como criar e expandir eventos narrativos via DataAssets. O sistema **funciona sem nada disso** — quando nenhum `UEventContentRegistry` é apontado, o `UEventSubsystem` carrega 5 eventos hardcoded no startup.

---

## 0. Quando criar assets

Crie eventos como DataAssets quando você quer:
- Mais que 5 eventos
- Editar valores sem rebuild de C++
- Modders/designers contribuírem
- Imagens, áudio, localização real

Para iteração rápida durante prototipação, mexer direto em `RegisterFallbackEvents` (em `Source/StrategosCore/Private/Events/EventSubsystem.cpp`) é mais rápido.

---

## 1. Estrutura de pastas

```
Content/Events/
  Conditions/   ← (vazio por agora; futuras Blueprint conditions)
  Effects/      ← (vazio; futuros effects custom em BP)
  Events/       ← UEventAsset (1 arquivo por evento)
  DA_EventRegistry  ← UEventContentRegistry agregando todos
```

---

## 2. Triggers canônicos

Strings exatas reconhecidas pelo subsystem:

| TriggerTag | Disparado por |
|---|---|
| `Time.Month` | A cada tick mensal, uma vez por nação |
| `Time.Year` | A cada tick anual, uma vez por nação |
| `Military.ArmyArrived` | Quando exército chega a nova província |
| `Economy.BuildingCompleted` | Quando obra termina |
| `Economy.BankruptcyImminent` | Quando dívida > 5× income mensal |
| `Chain` | Disparado por `Effect_FireEvent` (encadeamento) |

Adicionar trigger novo: dois passos em `EventSubsystem.cpp`:
1. Adicionar a constante no namespace `EventTriggers`
2. Subscribe ao delegate apropriado em `SubscribeToTriggers()` + handler chamando `DispatchTrigger`

---

## 3. Criar um UEventAsset

*Add → Miscellaneous → Data Asset → EventAsset*. Nomeie `E_<Nome>`.

### Campos obrigatórios

| Campo | Notas |
|---|---|
| `Id` | FName único, prefixo opcional por categoria. Ex.: `Economy.MarketPanic` |
| `Title` | Exibido no popup |
| `Description` | Multi-line; o que aconteceu |
| `Type` | Decision / Notification / Silent |
| `TriggerTag` | Uma das constantes acima |
| `MeanTimeToHappenMonths` | 0 = sempre que conditions passam; N = ~1/N chance |

### Conditions (Instanced array)

Adicione instâncias de `UEventCondition`. v1 vem com 3:

| Tipo | Campos | Uso típico |
|---|---|---|
| `TreasuryBelow` | NationId, Threshold | Crise fiscal, "preciso vender títulos" |
| `LoyaltyBelow` | NationId, Stratum, Threshold | Greve, revolta iminente |
| `HasGoodInStockpile` | NationId, GoodId, MinAmount | Festival se há comida, etc |

`NationId` vazio = usa a nação alvo do trigger (Context.SourceNationId).

### Effects (Instanced)

`AutoEffects` para Notification/Silent. `Choices[].Effects` para Decision.

| Tipo | Campos | Uso |
|---|---|---|
| `AddGold` | NationId, Amount | +/- Treasury |
| `AddPopLoyalty` | NationId, Stratum, Delta, bAllStrata | Mexer loyalty |
| `AddGoodsToStockpile` | NationId, Goods[] | Bens (Amount negativo drena) |
| `FireEvent` | EventId, TargetNationId | Chaining para narrativa multi-passo |

### Choices (apenas Decision)

Cada choice tem `Label`, `Tooltip` e `Effects[]`. **Pelo menos uma choice é obrigatória** — Decision sem choices vira Notification graceful (com log de aviso).

---

## 4. Registrar no registry

Crie `DA_EventRegistry` (UEventContentRegistry) em `Content/Events/`. Arraste todos os `E_*` para `Events`.

No `BP_StrategosGameMode` (em `BeginPlay` do BP):
1. Get World Subsystem → `EventSubsystem`
2. Set Content Registry → `DA_EventRegistry`

A partir desse ponto, o subsystem **substitui** os 5 fallback events pelos do registry. Para misturar (fallback + custom), edite `RebuildIndex` em C++ — atualmente é XOR (registry OR fallback).

---

## 5. Exemplo: criando um evento de epidemia

Vamos modelar um evento de doença nas províncias, como demo.

### Asset: `E_PlagueOutbreak`
- **Type**: Notification
- **TriggerTag**: `Time.Year`
- **MTTH**: 24 (≈ uma vez a cada 24 anos por nação que satisfaz conditions)
- **Conditions**:
  - `TreasuryBelow` (NationId vazio, Threshold 200) — só atinge países pobres
- **AutoEffects**:
  - `AddPopLoyalty` (Stratum: Laborer, Delta: -0.15, bAllStrata: false)
  - `AddGoodsToStockpile` (Goods: [{Bread, -100}, {Garments, -50}])
  - `AddGold` (Amount: -100)

Salve. No próximo `Time.Year`, países com Treasury < 200 têm 1/24 chance de receber a praga.

---

## 6. Decision com chain

Vamos modelar um arco em 2 eventos: rumor de revolta → revolta efetivada.

### `E_RebellionRumor` (Decision, Time.Month, MTTH 36)
Conditions: `LoyaltyBelow` (Workers, 0.3)

Choice 1 — "Send agents to investigate":
- `AddGold` (-50)
- `FireEvent` (EventId: `E_RebellionResolved`, TargetNationId: vazio)

Choice 2 — "Ignore the rumors":
- `FireEvent` (EventId: `E_RebellionFull`)

### `E_RebellionResolved` (Notification)
Conditions: vazio (chain só dispara se choice 1)
AutoEffects: `AddPopLoyalty` (Workers, +0.10)

### `E_RebellionFull` (Notification)
AutoEffects: `AddPopLoyalty` (Workers, -0.30, bAllStrata=true), `AddGold` (-200)

Resultado: o player percebe que ignorar deu ruim. Mecânica de aprendizado.

---

## 7. Smoke test

Em runtime (com fallback ou DA_EventRegistry):

1. Logs em `Output Log`:
   ```
   EventSubsystem: registered 5 fallback events.
   EventSubsystem indexed 5 events across 2 trigger tags.
   ```
   ou (com registry):
   ```
   EventSubsystem indexed 12 events across 4 trigger tags.
   ```

2. Avance vários meses. Eventos aparecem nos logs:
   ```
   Event fired: BountifulHarvest on Albion
   Decision FestivalPetition queued for player nation Albion
   ```

3. Para testar Decision UI: implemente `OnDecisionEnqueued` no `WBP_HUD` mostrando popup. Use `GetTopPendingDecision` para popular o widget. Botões chamam `ResolvePendingDecision(EventId, ChoiceIndex)`.

4. Para testar Notification: `OnNotificationFired` recebe (EventId, Title, Description) — cria toast/popup transitório.

---

## 8. Determinismo

Eventos são determinísticos:
- Ordem de iteração dentro de uma trigger é alfabética por Id
- MTTH usa FRandomStream seeded com `(NationId, EventId, Date.Ticks)`
- AI auto-resolve usa hash determinístico para escolher choice

Resultado: mesmo save → mesma sequência de eventos, mesmas escolhas da IA. Crítico para multiplayer (Etapa 4) e replay.

---

## 9. Tuning rápido

| Sintoma | Onde olhar |
|---|---|
| Nenhum evento dispara | Verifique TriggerTag exato (case-sensitive). Confira se DA_EventRegistry foi setado |
| Evento dispara demais | Aumente MeanTimeToHappenMonths ou aperte conditions |
| Decision nunca aparece | Conditions falhando, ou Decision Conditions são para AI nation |
| FireEvent silencioso | Nada feito até wire (commit 6); ver log `Effect_FireEvent stub` |
| AI sempre escolhe mesma choice | Esperado — hash determinístico. Adicionar variabilidade vem com UAIDirectorSubsystem |

---

## 10. Roadmap de extensão

Conditions/effects que entram conforme outros subsistemas chegam:

- **`Condition_IsAtWar`**, **`Effect_DeclareWar`** — quando UDiplomacySubsystem existir (Etapa 2 ainda)
- **`Condition_HasTech`**, **`Effect_GrantTech`** — UProgressSubsystem (Etapa 3)
- **`Condition_OwnsProvince`**, **`Effect_TransferProvince`** — interage com Diplomacy
- **`Effect_SpawnArmy`**, **`Effect_DestroyBuilding`** — disponíveis hoje, fáceis de adicionar
- **`Condition_LedByArchetype`** — testa CurrentLeader.Archetype; tornaria eventos personalidade-específicos

Padrão: cada nova condition/effect = 1 header + 1 cpp + entry no registry. Sem mudança em UEventSubsystem.

---

## 11. Modding

Modders podem:
1. Criar UEventAsset próprios em `Content/Mods/<ModName>/Events/`
2. Criar `DA_EventRegistry_Mod` apontando seus + os vanilla
3. Substituir o registry no GameMode via Blueprint setting

Não há invasão de código — toda a extensibilidade vive no plugin de DataAssets.
