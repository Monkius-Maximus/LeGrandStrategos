#pragma once

#include "CoreMinimal.h"
#include "BuildingState.generated.h"

/**
 * EBuildingState — Estado do ciclo de vida de um UBuilding (doc §7).
 *
 * UnderConstruction: ainda não produz; consome ConstructionCost ao longo
 *                    de ConstructionDays e então transita para Operational.
 * Operational      : produz normalmente seguindo PM + FResourceRoutingPolicy.
 * SurvivalMode     : autônomo demitiu operários, liquida estoque < custo.
 *                    Última tentativa de evitar Ruined (doc §7).
 * Ruined           : faliu, demitiu todos, ocupa slot mas não produz.
 *                    Player deve pagar para demolir ou nacionalizar.
 * Nationalized     : transição forçada de Ruined → Operational como State.
 *                    Custo abrupto no tesouro; emprega menos no primeiro tick.
 */
UENUM(BlueprintType)
enum class EBuildingState : uint8
{
	UnderConstruction	UMETA(DisplayName = "Under Construction"),
	Operational			UMETA(DisplayName = "Operational"),
	SurvivalMode		UMETA(DisplayName = "Survival Mode"),
	Ruined				UMETA(DisplayName = "Ruined"),
	Nationalized		UMETA(DisplayName = "Nationalized")
};

/**
 * FBuildingRuinState — Snapshot dos custos pendentes de uma ruína.
 *
 * Carregado em UBuilding quando State == Ruined. DemolitionCost e
 * NationalizationCost são calculados no momento do colapso (em função de
 * Level + tipo) e ficam fixos até a ação ser executada — para que o
 * jogador veja o trade-off na UI sem reflutuação a cada tick.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FBuildingRuinState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Ruin")
	int32 MonthsSinceCollapse = 0;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Ruin", meta = (ClampMin = "0.0"))
	float DemolitionCost = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Ruin", meta = (ClampMin = "0.0"))
	float NationalizationCost = 0.f;
};
