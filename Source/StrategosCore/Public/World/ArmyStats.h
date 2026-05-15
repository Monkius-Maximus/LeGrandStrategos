#pragma once

#include "CoreMinimal.h"
#include "ArmyStats.generated.h"

/**
 * FArmyStats — bloco numérico de uma unidade. Hard stats principais (ATQ/DEF/MOB/MOR/ORG/SUP)
 * são funcionais já no MVP; secundárias (ALC/PREC/SUPR/REC/CST) ficam cosméticas até o
 * BattleResolver entrar em cena.
 *
 * Convenções de range: 0–100 para os 6 principais; ALC em tiles; PREC em [0..1].
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FArmyStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 ATQ = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 DEF = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 MOB = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 MOR = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 ORG = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 SUP = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 ALC = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) float PREC = 0.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 SUPR = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 REC = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 CST = 0;
};

UENUM(BlueprintType)
enum class EUnitState : uint8
{
	Ready			UMETA(DisplayName = "Pronta"),
	InCombat		UMETA(DisplayName = "Em combate"),
	Damaged			UMETA(DisplayName = "Danificada"),
	Disorganized	UMETA(DisplayName = "Desorganizada"),
	Retreated		UMETA(DisplayName = "Retirada")
};

/**
 * FArmyModifier — buff/debuff temporário aplicado a uma unidade. Exibido como chip
 * verde (positivo) ou vermelho (negativo) na carta detalhada.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FArmyModifier
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, EditAnywhere) FName Id;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) FText Label;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) float Value = 0.f;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) int32 TurnsRemaining = 0;
	UPROPERTY(BlueprintReadWrite, EditAnywhere) bool bPositive = true;
};
