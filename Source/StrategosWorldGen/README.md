# StrategosWorldGen

> **Sub-projeto experimental de estudo.** Não faz parte do MVP do Le Grand Strategos.
> Implementa o pipeline descrito em `WORLDGEN_PIPELINE_REFERENCE.md`: a versão
> "como eu faria do zero, sem o FMG".

Pipeline de geração procedural de mundos 2D: pontos → Voronoi → heightmap →
coastline → clima → biomas → rios → render de debug. Saída é um conjunto de
`FWorldCell` classificadas, que um passo posterior (estados, províncias) pode
consumir.

## Por que um módulo separado

- Dependência third-party (`delaunator-cpp`), isolada de StrategosCore.
- Usa `bEnableExceptions = true` (delaunator lança `std::runtime_error`).
- Pode ser excluído do shipping se a geração for offline-only.

## Estágios (mapeamento com o documento de referência)

| Estágio | Arquivo | Descrição |
|---------|---------|-----------|
| 1. Sampling | `Pipeline/PoissonDiscSampler` | Bridson 2007 (Poisson-disc) |
| 2. Voronoi | `Pipeline/VoronoiBuilder` | delaunator + topologia + Lloyd |
| 3. Heightmap | `Pipeline/HeightmapGenerator` | Blobs + templates + Perlin |
| 4. Coastline | `Pipeline/ClimateGenerator` | `DetectWaterAndCoast` |
| 5. Temperatura | `Pipeline/ClimateGenerator` | Latitude + lapse rate |
| 6. Vento | `Pipeline/ClimateGenerator` | `GetWindForCell` (bandas) |
| 7. Precipitação | `Pipeline/ClimateGenerator` | Marcha de nuvens upwind |
| 8. Biomas | `Pipeline/BiomeClassifier` | Matriz Whittaker |
| 9. Rios | `Pipeline/RiverTracer` | Downhill graph (rascunho) |
| 11. Render | `Pipeline/WorldGenDebugRenderer` | Opção A (UTexture2D) |

`Pipeline/WorldGenPipeline` orquestra a ordem; `WorldGenSubsystem` é o entry
point (BlueprintCallable); `WorldGenDataAsset` guarda presets de parâmetros.

## Determinismo

Um único `FRandomStream` (em `FWorldGenContext`) propaga por todos os estágios.
Nenhum estágio chama `FMath::RandRange` — sempre `Stream.*`. Mesma seed +
mesmos `FWorldGenParams` = mesmo mundo.

## Uso (C++)

```cpp
UWorldGenSubsystem* Gen = GameInstance->GetSubsystem<UWorldGenSubsystem>();

FWorldGenParams Params;
Params.Seed = 1337;
Params.MapSize = FIntPoint(1024, 1024);
Params.Template = EWorldTemplate::Continent;

if (Gen->GenerateWorld(Params))
{
    UTexture2D* Debug = Gen->RenderDebugTexture(EWorldGenRenderMode::Biomes);
    // ... exibir Debug num widget para inspeção visual
}
```

Para validar estágio a estágio (Seção 16 do documento), troque o
`EWorldGenRenderMode`: `RandomCells` → `Height` → `Coast` → `Temperature`
→ `Precipitation` → `Biomes`.

## Teste rápido sem código: `AWorldGenDebugActor`

Arraste um **WorldGenDebugActor** para um nível, ajuste `Params`/`Mode` no
painel Details e clique **"Generate And Export"** (botão `CallInEditor`, não
precisa dar Play) ou dê Play. Os PNGs de debug saem em
`<Projeto>/Saved/WorldGen/`. Com `bExportAllModes = true` sai um por estágio.

## Limitações conhecidas (escopo de estudo)

- Voronoi: células de borda têm polígono aberto; marcadas `bIsBorder` e
  forçadas a água, não clipadas contra o retângulo do mapa.
- Rios: sem lagos, sem pit-filling (Planchon-Darboux), sem larguras variáveis.
- Clima: sem feedback nem sazonalidade.
- `FWorldGenResult` é runtime-only (contém arrays não reflectíveis); ainda
  não há serialização para asset persistido.

## Third-party

`ThirdParty/delaunator-cpp/` — header-only, MIT (ver `LICENSE`).
