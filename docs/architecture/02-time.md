# 02 — UTimeSubsystem

O subsistema de tempo é o **metrônomo da simulação**. Todos os outros subsistemas que precisam evoluir no tempo se inscrevem nele. Não usar `Tick()` da Actor para simulação de jogo — frame rate diferente quebra balanceamento.

---

## 1. Definição

```cpp
class UTimeSubsystem : public UGameInstanceSubsystem
{
    int64 CurrentTickDay;        // dias desde início
    float SimSpeed;              // 0=paused, 1..5=velocidades
    DECLARE_EVENT(UTimeSubsystem, FOnDay)   OnDay;
    DECLARE_EVENT(UTimeSubsystem, FOnMonth) OnMonth;
    DECLARE_EVENT(UTimeSubsystem, FOnYear)  OnYear;
};
```

---

## 2. Granularidade dos Subsistemas

Subsistemas se inscrevem na granularidade que precisam:

| Subsistema | Inscrição | Justificativa |
|---|---|---|
| Economia | `OnDay` (ou `OnWeek`) | Produção/consumo precisa cadência fina |
| Diplomacia | `OnMonth` | Decisões diplomáticas são lentas |
| Tecnologia | `OnDay` | Pesquisa progride continuamente |
| População | `OnMonth` | Crescimento e migração são lentos |
| Militar (movimento) | `OnDay` | Movimento de exército por dia |
| Política | `OnMonth` | Reformas, militância, eleições |

---

## 3. Pontos de Atenção

- Nunca use `Tick()` da Actor para simulação de jogo. **Frame rate diferente quebra balanceamento**.
- `SimSpeed` deve ser configurável: 0 (pausado), 1 (lento), 2 (normal), 3 (rápido), 4 (muito rápido), 5 (turbo).
- Quando o `UGameFlowSubsystem` muda para `Paused/Event/Battle`, o `TimeSubsystem` deve setar `SimSpeed = 0` automaticamente.
- O **calendário** (data atual visível para o jogador) deriva de `CurrentTickDay` somado a uma data de início (ex: 1836-01-01 para cenário vitoriano clássico).
- Permita **pausas automáticas** em eventos críticos (declaração de guerra, evento decisão, fim de pesquisa) — opcional, configurável pelo jogador.

---

## 4. Determinismo

- O tick é movido por `DeltaTime` real, mas a **simulação avança em unidades de "dia"** discretas. Frações de dia são acumuladas e disparadas quando completam um dia inteiro.
- Isso garante que dois saves carregados em PCs diferentes evoluam **identicamente** se o RNG e a ordem de iteração forem estáveis.
- O `CurrentTickDay` faz parte do save. Replays funcionam comparando `CurrentTickDay` + RNG seed.

---

## 5. Eventos Emitidos

```cpp
FOnDay   OnDay;     // todo dia simulado
FOnWeek  OnWeek;    // a cada 7 dias
FOnMonth OnMonth;   // todo mês simulado
FOnYear  OnYear;    // todo ano simulado
```

> A cadência exata pode variar conforme o cenário (ex: ticks "diários" podem ser semanais para cenário antigo de longa duração). Mantenha a API estável e ajuste a constante interna.

---

## 6. Integração com FSM

O `TimeSubsystem` é um listener do `UGameFlowSubsystem`. Quando o estado muda:

| Estado FSM | SimSpeed |
|---|---|
| `MainMenu` | 0 |
| `Loading` | 0 |
| `Running` | restaurado para o valor anterior do jogador |
| `Paused` | 0 |
| `Battle` | 0 (mapa-mundi não evolui durante batalha tática) |
| `Event` | 0 |
| `GameOver` | 0 |
