# Le Grand Strategos

Um projeto de desenvolvimento de um jogo de Grand Strategy que une as principais mecânicas de jogos do gênero — inspirado em **Victoria 2/3**, **Civilization**, **Humankind** e **Romance of the Three Kingdoms** — com um toque mais atualizado para os dias atuais. Combina simulação macro profunda (POPs, mercados regionais, ideologias, diplomacia com esferas de influência) com uma camada tática de batalhas baseada em comandantes, regimentos equipados e um sistema de cartas de decisão.

## Engine

Unreal Engine, com simulação em C++ (Camadas 1–3) e UI/scripting em Blueprint (Camada 5).

## Documentação

A arquitetura de sistemas está documentada em [`docs/`](docs/README.md), organizada por subsistema. Comece pelo [índice](docs/README.md) ou direto pela [visão geral](docs/architecture/00-overview.md).

Resumo dos subsistemas detalhados:

- **Fundação** — FSM principal, tempo simulado, event bus
- **Combate e Unidades** — batalha tática com cartas, sistema de unidades por composição
- **Simulação Macro** — economia (POPs + mercados), tecnologia (eras + vitórias)
- **Eventos e Sociedade** — eventos narrativos, política interna, diplomacia
- **Inteligência e Estratégia** — IA nacional, sistema militar estratégico

Plano de implementação consolidado em [`docs/architecture/99-implementation-roadmap.md`](docs/architecture/99-implementation-roadmap.md).
