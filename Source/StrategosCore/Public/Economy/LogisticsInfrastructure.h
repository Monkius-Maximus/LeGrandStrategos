#pragma once

#include "CoreMinimal.h"
#include "LogisticsInfrastructure.generated.h"

/**
 * EInfrastructureType — Tipos de infraestrutura logística que somam
 * Largura de Banda à província (doc §5).
 *
 * DirtRoad : default; baixa capacidade, força dependência do mercado local.
 * PavedRoad: pedra/macadame; +banda intermediária, baixo CAPEX.
 * Railway  : ferrovia; salto grande de banda, funde local↔global.
 * RiverPort: cais fluvial; só se EWaterAccessType >= MinorRiver.
 * SeaPort  : porto marítimo; só se EWaterAccessType == Coastal.
 * Canal    : obra de engenharia (Suez/Panamá-like); buff regional.
 */
UENUM(BlueprintType)
enum class EInfrastructureType : uint8
{
	DirtRoad	UMETA(DisplayName = "Dirt Road"),
	PavedRoad	UMETA(DisplayName = "Paved Road"),
	Railway		UMETA(DisplayName = "Railway"),
	RiverPort	UMETA(DisplayName = "River Port"),
	SeaPort		UMETA(DisplayName = "Sea Port"),
	Canal		UMETA(DisplayName = "Canal")
};

/**
 * FInfrastructureLevel — Nível atual de uma infraestrutura instalada.
 *
 * Capacity expressa em "unidades de carga / mês". Multiplicadores de
 * tipo (ex.: ferrovia x10 vs estrada) ficam em URGOTemplateAsset-equivalent
 * para infraestrutura (TBD; por ora hardcoded em UEconomySubsystem).
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FInfrastructureLevel
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Infra")
	EInfrastructureType Type = EInfrastructureType::DirtRoad;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Infra", meta = (ClampMin = "0"))
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Infra", meta = (ClampMin = "0.0"))
	float CapacityPerMonth = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Infra", meta = (ClampMin = "0.0"))
	float MaintenancePerMonth = 1.0f;
};

/**
 * FLogisticsBandwidth — Soma efetiva de Largura de Banda de uma província
 * em um dado tick.
 *
 * IncomingCapacity é o que pode chegar ao mercado local sem custo
 * adicional de "estouro de banda". OutgoingCapacity é o que pode ser
 * exportado a hubs / GlobalMarket. ConnectedHubId aponta o hub atual de
 * conexão dominante (ver UTradeHub). UsedThisTick é cache populado pelo
 * EconomySubsystem para a UI mostrar congestionamento.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FLogisticsBandwidth
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Bandwidth", meta = (ClampMin = "0.0"))
	float IncomingCapacity = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Bandwidth", meta = (ClampMin = "0.0"))
	float OutgoingCapacity = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Bandwidth")
	FName ConnectedHubId;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Bandwidth|Cache", meta = (ClampMin = "0.0"))
	float UsedIncomingThisTick = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Bandwidth|Cache", meta = (ClampMin = "0.0"))
	float UsedOutgoingThisTick = 0.f;

	/** Lista bruta de infra instalada que somou para o cálculo das capacidades. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Bandwidth")
	TArray<FInfrastructureLevel> InstalledInfrastructure;
};
