# ADR-0001 — `Strategos.Sim` nao conhece Godot

**Status:** ACEITO
**Data:** 2026-09-06
**Contexto:** doc `70-arquitetura-modular-e-godot.md`; tensao herdada do LGP

---

## Contexto

O LGP identificou uma tensao arquitetural e nao a resolveu: o objetivo de ter uma
assembly de simulacao em C# puro conflitava com a conveniencia de usar os tipos
`Resource` e `StringName` do Godot como definicoes canonicas de dado.

A tensao e real. Usar `Resource` da, de graca, serializacao, o inspector do editor
e o sistema de import do Godot. Recusa-los significa escrever carregamento de dado
a mao.

Com a decisao de engine fechada em Godot (2026-09-06), a pergunta virou bloqueante:
todo dado do jogo — 31 goods, ladders de PM, `UInterestGroupAsset`, `ULawAsset` —
precisa de uma forma canonica, e essa escolha e cara de reverter depois.

## Decisao

**`Strategos.Sim` nao referencia nenhum assembly Godot.** Nem `GodotSharp`, nem
`Godot.NET.Sdk`, nem transitivamente.

Consequencias diretas:

- Identificadores usam `SimId`, nao `StringName` nem `FName`.
- Aleatoriedade usa `DeterministicRng`, nao `RandomNumberGenerator` do Godot.
- Definicoes de dado sao tipos C# puros (POCOs), nao `Resource`.
- `Strategos.Game` carrega os dados e **adapta** para os tipos do Sim na fronteira.
  A adaptacao acontece na entrada; nada de tipo Godot atravessa para dentro.

A fronteira e verificada por teste automatizado (`tests/Strategos.Sim.Tests`,
secao ADR-0001), que inspeciona os assemblies referenciados e falha o build se
algum `Godot*` aparecer. **A decisao e um invariante executavel, nao uma recomendacao.**

## Racional

1. **E o objetivo declarado do LGP.** Validar a arquitetura de subsistemas
   engine-agnostica e uma das duas metas do projeto. Usar `Resource` como definicao
   canonica derrota essa meta silenciosamente.
2. **Determinismo e contrato de save.** O projeto exige que uma rolagem resolvida
   nao mude apos reload. Isso exige controle total sobre o estado do RNG e sobre a
   ordem de serializacao — controle que se perde ao delegar para o ciclo de vida de
   `Resource`.
3. **Testabilidade sem editor.** O auto-resolver de batalha roda headless e usa o
   mesmo engine da batalha jogada (doc `60`). Se a simulacao depende de tipos Godot,
   "headless" passa a significar "com o runtime do Godot carregado", e o custo de
   cada teste sobe de milissegundos para segundos.
4. **A migracao UE5 -> Godot ja custou este projeto uma vez.** O que doeu nao foi a
   engine; foi o acoplamento a ela. Pagar o preco de novo em uma terceira migracao
   nao e aceitavel.
5. **Separacao de responsabilidades** (principio 12 do projeto): regra de simulacao
   e apresentacao sao preocupacoes distintas e devem ser compilaveis em separado.

## Consequencias

**Custo aceito:**
- Carregamento de dado escrito a mao (JSON ou binario proprio), sem inspector do Godot
  para editar DataAssets. Uma ferramenta de autoria propria pode ser necessaria depois.
- Uma camada de adaptacao entre o formato em disco e os tipos do Sim.

**Ganho:**
- A simulacao roda em teste, em CLI e em CI sem Godot instalado.
- Trocar de engine de novo custa reescrever `Strategos.Game`, nao o jogo.
- O import do FMG e uma CLI .NET, nao um tool script preso ao editor.

**Nao decidido aqui:** o formato em disco dos dados (JSON, MessagePack, binario
proprio). Fica para o ADR-0002, quando o primeiro DataAsset real existir.

## Reversao

Esta decisao se reverte apagando o teste de fronteira e adicionando a referencia.
O sinal de que ela estava errada seria: a camada de adaptacao crescer mais que o
codigo de simulacao que ela alimenta. Se isso acontecer, reabrir.
