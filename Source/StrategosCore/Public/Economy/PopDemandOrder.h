#pragma once

#include "CoreMinimal.h"
#include "Economy/PopStratum.h"
#include "PopDemandOrder.generated.h"

/**
 * EDemandFulfillmentSource — De onde um pedido foi atendido nesse tick.
 *
 * Drives both the UI tooltip ("este pão veio do mercado local") e o
 * cálculo do custo logístico que o pop pagou. Ordem do enum reflete a
 * cascata de §6: warehouse → local → global → unmet.
 */
UENUM(BlueprintType)
enum class EDemandFulfillmentSource : uint8
{
	CityWarehouse	UMETA(DisplayName = "City Warehouse"),
	LocalMarket		UMETA(DisplayName = "Local Market"),
	GlobalMarket	UMETA(DisplayName = "Global Market (Imported)"),
	Unmet			UMETA(DisplayName = "Unmet")
};

/**
 * FPopDemandOrder — Pedido aberto por um POP num tick econômico.
 *
 * Fluxo (doc §6):
 *  1. POP tenta comprar em CityWarehouse (custo 0 de frete).
 *  2. Sobra abre pedido no LocalMarket.
 *  3. Sobra cascata para GlobalMarket somando custo de frete e tarifa de hub.
 *
 * O subsystem cria um destes por (PopStratum × GoodId × ProvinceId) por
 * tick, resolve em ordem, e popula RequestedAmount/FulfilledAmount para a
 * UI mostrar shortage exato. Não é serializado: regenerado a cada tick.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FPopDemandOrder
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Demand")
	FName ProvinceId;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Demand")
	EPopStratum Stratum = EPopStratum::Laborer;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Demand")
	FName GoodId;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Demand", meta = (ClampMin = "0.0"))
	float RequestedAmount = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Demand", meta = (ClampMin = "0.0"))
	float FulfilledAmount = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Demand")
	EDemandFulfillmentSource Source = EDemandFulfillmentSource::Unmet;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Demand", meta = (ClampMin = "0.0"))
	float UnitPricePaid = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Demand", meta = (ClampMin = "0.0"))
	float LogisticsSurcharge = 0.f;
};
