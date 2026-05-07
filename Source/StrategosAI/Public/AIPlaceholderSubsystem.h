#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "AIPlaceholderSubsystem.generated.h"

class UWorldState;
class UNation;
class UArmy;
class UTimeSubsystem;
class UMilitarySubsystem;
class UEconomySubsystem;

/**
 * UAIPlaceholderSubsystem — Comportamento das nações IA na Etapa 1.
 *
 * Não é IA "de verdade": é uma máquina de regras dirigida pelo Archetype
 * do líder atual, suficiente para que nações IA mostrem sinais de vida
 * sem gastar engenharia em algo que será substituído pelo
 * UAIDirectorSubsystem (Etapa 3).
 *
 * Ciclos:
 *  - OnYearTick: 5% de chance de morte/queda do líder. Sucessor é
 *    sorteado pelos pesos UNation::ArchetypeAffinity (que já incluem
 *    bônus das NationalIdeas).
 *  - OnMonthTick: para cada nação não-jogador, executa
 *    RunArchetypeBehavior(Nation) conforme o arquétipo do líder.
 *
 * Determinismo: todas as decisões usam um FRandomStream seeded por
 * (NationId, Year, Month/0). Mesma seed → mesma escolha.
 */
UCLASS()
class STRATEGOSAI_API UAIPlaceholderSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

private:
	UFUNCTION()
	void HandleMonthTick(FDateTime CurrentDate);

	UFUNCTION()
	void HandleYearTick(FDateTime CurrentDate);

	void RunArchetypeBehavior(UNation& Nation, const FDateTime& CurrentDate);

	void Behavior_Militarist(UNation& Nation, FRandomStream& RNG);
	void Behavior_Diplomat(UNation& Nation, FRandomStream& RNG);
	void Behavior_Pragmatist(UNation& Nation, FRandomStream& RNG);
	void Behavior_Merchant(UNation& Nation, FRandomStream& RNG);
	// Religious / Intellectual: ainda no-op até Politics/Progress.

	/** Hook econômico chamado independente do arquétipo (preferências base). */
	void EconomyBehavior_Common(UNation& Nation, FRandomStream& RNG);

	void RollLeaderSuccession(UNation& Nation, int32 Year);

	UWorldState* ResolveWorldState() const;
	UTimeSubsystem* ResolveTime() const;
	UMilitarySubsystem* ResolveMilitary() const;
	UEconomySubsystem* ResolveEconomy() const;

	static FRandomStream MakeStream(FName NationId, int32 Year, int32 Month);

	// Probabilidade anual de troca de líder.
	static constexpr float LeaderTurnoverChance = 0.05f;

	// Para Militarist/Pragmatist: probabilidade mensal de mover um exército.
	static constexpr float MonthlyMoveChance = 0.30f;
};
