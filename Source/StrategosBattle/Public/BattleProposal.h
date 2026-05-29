#pragma once

#include "CoreMinimal.h"
#include "BattleTypes.h"
#include "BattleProposal.generated.h"

/**
 * FBattleProposal — DTO de entrada da batalha.
 * Produzido por UMilitarySubsystem quando dois exércitos se engajam.
 * Consumido por UBattleSubsystem (tático) ou UBattleResolverService (auto-resolve).
 */
USTRUCT(BlueprintType)
struct STRATEGOSBATTLE_API FBattleProposal
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite) int32 ProvinceId = -1;
	UPROPERTY(BlueprintReadWrite) EBattleType Type = EBattleType::DirectClash;
	UPROPERTY(BlueprintReadWrite) int32 AttackerNationId = -1;
	UPROPERTY(BlueprintReadWrite) int32 DefenderNationId = -1;
	UPROPERTY(BlueprintReadWrite) TArray<FName> AttackerArmyIds;
	UPROPERTY(BlueprintReadWrite) TArray<FName> DefenderArmyIds;
	UPROPERTY(BlueprintReadWrite) EBattleTerrain Terrain = EBattleTerrain::Plains;
	UPROPERTY(BlueprintReadWrite) EBattleWeather Weather = EBattleWeather::Clear;
};

/**
 * FBattleResult — DTO de saída da batalha.
 * UMilitarySubsystem::ApplyBattleResult() é o único ponto que muta o WorldState.
 */
USTRUCT(BlueprintType)
struct STRATEGOSBATTLE_API FBattleResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FGuid BattleId;
	UPROPERTY(BlueprintReadOnly) EBattleOutcome Outcome = EBattleOutcome::Stalemate;

	/** Quanto manpower cada exército perdeu (ArmyId → soldados perdidos). */
	UPROPERTY(BlueprintReadOnly) TMap<FName, int32> ArmyStrengthLost;

	UPROPERTY(BlueprintReadOnly) TArray<FName> CapturedCommanders;
	UPROPERTY(BlueprintReadOnly) TArray<FName> KilledCommanders;

	UPROPERTY(BlueprintReadOnly) float MoraleHitAttacker = 0.f;
	UPROPERTY(BlueprintReadOnly) float MoraleHitDefender = 0.f;
	UPROPERTY(BlueprintReadOnly) float SupplyConsumed = 0.f;

	/** +1 = atacante ganhou controle, -1 = defensor manteve, 0 = empate. */
	UPROPERTY(BlueprintReadOnly) int32 ProvinceControlChange = 0;

	UPROPERTY(BlueprintReadOnly) TArray<FBattleLogEntry> CombatLog;
};
