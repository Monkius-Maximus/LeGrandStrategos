# Prompt — Sessão 1: Geografia da Província

> Cole este prompt no Claude Code rodando dentro do repositório. Escopo travado: **só dados geográficos de província e persistência**. Nada de slots, elegibilidade, bens ou tech tier.

---

## Contexto

Projeto: Le Grand Strategos, grand strategy em UE5 (C++), era 1820–1880. Economy v1 já implementado (`UProductionMethodAsset`, mercado, tick de 9 fases, auto-invest da Bourgeoisie). Esta é a Sessão 1 do Sistema de Recursos e Produção — ver `docs/architecture/40-resources-and-production.md`.

Esta sessão entrega **apenas a Camada 1**: a geografia estática que será consumida pelas camadas seguintes. Sem comportamento ainda — só dado + persistência + acesso.

## Escopo (fazer exatamente isto)

1. **Criar quatro tipos geográficos** no header apropriado de `UProvince` (ou no arquivo de tipos compartilhados do módulo de mapa, seguindo onde os enums de província já vivem):
   - `enum class ETerrainTopography : uint8` → `Flatland, Hills, Mountains, Plateau, Wetlands, Coastal`
   - `enum class EClimateZone : uint8` → `Tropical, Mediterranean, Continental, Arid, Arctic, Oceanic`
   - `enum class EVegetationCover : uint8` → `DenseForest, LightForest, Grassland, Wetland, Desert, Tundra`
   - `struct FHydrography` com flags `bool`: `bHasMinorRiver, bHasNavigableRiver, bHasLake, bHasAquifer, bIsCoastal` (defaults `false`).
   - Todos com `UENUM(BlueprintType)` / `USTRUCT(BlueprintType)` e `UPROPERTY` nos campos, seguindo o padrão dos enums já existentes em `UProvince`.

2. **Adicionar os campos em `UProvince`** como `UPROPERTY`:
   - `ETerrainTopography Topography`
   - `EClimateZone Climate`
   - `EVegetationCover Vegetation`
   - `FHydrography Hydrography`
   - Defaults sensatos (ex.: `Flatland`, `Continental`, `Grassland`, hidrografia toda `false`).

3. **Persistência**: serializar os quatro no save/load de província. Subir `SaveVersion` para **6**. Garantir migração: saves versão ≤5 carregam com os defaults acima sem crashar (campo novo assume default).

4. **Acessores** simples e read-only para as camadas futuras consumirem: getters `const`. Sem setters públicos por agora (geografia é estática; quem popula é a geração de mapa, não gameplay).

## Fora de escopo (NÃO tocar)

- ❌ Slots, elegibilidade, recurso principal, tech tier, bens — sessões futuras.
- ❌ Qualquer lógica de produção, preço, mercado (Economy v1 fica intocado).
- ❌ Renderização / visual de relevo no mapa.
- ❌ Limpar floresta ou qualquer mutação de geografia em runtime.
- ❌ Modificadores de produção por terreno (Sessão 7).
- ❌ Não criar sistema de constantes/balanceamento ainda — só os enums e o struct.

## Convenções a respeitar (do CLAUDE.md)

- Código em inglês; comentários, commit e qualquer doc em PT-BR.
- Naming: enums `E`, structs `F`, UObjects `U`, BuildingType `BT_`.
- DataAsset com fallback hardcoded **não se aplica aqui** (não há DataAsset nesta sessão).
- Determinismo: nada de aleatório nesta sessão (geografia vem do mapa).
- Proibido: `LoadSynchronous` em tick; `AddDynamic` sem `UFUNCTION`; depender de ordem de iteração de `TMap`.

## Critério de pronto

- [ ] Compila sem warning novo.
- [ ] Os três enums + struct existem com `UENUM`/`USTRUCT` `BlueprintType` e aparecem no editor.
- [ ] `UProvince` tem os quatro campos com defaults, visíveis no editor.
- [ ] Save versão 6 grava e relê os quatro campos corretamente (smoke test: setar valores não-default numa província via código de teste, salvar, recarregar, conferir igualdade).
- [ ] Save versão ≤5 carrega sem crash, província assume defaults.
- [ ] Commit em PT-BR no padrão do repo, ex.: `feat(recursos): geografia de provincia (topografia/clima/vegetacao/hidrografia) + SaveVersion 6`.

## Armadilhas conhecidas

- Ao bumpar `SaveVersion`, conferir TODOS os pontos que comparam versão (não só o write). Migração de save é onde mais quebra.
- `FHydrography` precisa de `USTRUCT` com `GENERATED_BODY()` e os bools como `UPROPERTY`, senão não serializa via reflection.
- Se os enums de província hoje vivem num header específico, manter os novos no mesmo lugar — não criar header novo só pra isso (evitar dispersão).
- Não adicionar `UFUNCTION(BlueprintCallable)` nos getters a menos que já seja o padrão dos acessores existentes de `UProvince`.

## Ao terminar

Reportar: arquivos tocados, como ficou a migração de save, e o resultado do smoke test. Não avançar para slots — a Sessão 2 é prompt separado.
