#pragma once

#include "CoreMinimal.h"
#include "MarketDestination.generated.h"

/**
 * EMarketDestination — As 4 rotas possíveis para a produção de um prédio
 * (ver doc §4).
 *
 * StateStockpile  : reserva estratégica para guerra/manipulação de preço.
 * LocalMarket     : default sem infraestrutura — alimenta a própria província.
 * GlobalMarket    : escoa via hub de comércio buscando melhor margem.
 * PrivateWarehouse: buffer privado vs. embargo/flutuação artificial.
 */
UENUM(BlueprintType)
enum class EMarketDestination : uint8
{
	StateStockpile		UMETA(DisplayName = "State Stockpile"),
	LocalMarket			UMETA(DisplayName = "Local Market"),
	GlobalMarket		UMETA(DisplayName = "Global Market"),
	PrivateWarehouse	UMETA(DisplayName = "Private Warehouse")
};

/**
 * FResourceRoutingPolicy — Configuração do destino da produção de um
 * UBuilding individual.
 *
 * A política é editável pelo player apenas em prédios de posse estatal;
 * prédios privados (Bourgeoisie) decidem automaticamente em função de
 * spread de preço e capacidade de Largura de Banda disponível.
 *
 * Os 4 floats são frações de saída e devem somar 1.0 (clampeado em runtime).
 * Quando não há infraestrutura suficiente para uma rota (ex.: GlobalMarket
 * sem porto/ferrovia conectando), o EconomySubsystem reverte a fração
 * estourada para LocalMarket no fechamento do tick.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FResourceRoutingPolicy
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ShareToStateStockpile = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ShareToLocalMarket = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ShareToGlobalMarket = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routing", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ShareToPrivateWarehouse = 0.0f;

	/** Se true, o EconomySubsystem ignora o policy e usa autopilot AI (lucro). */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Routing")
	bool bAutomated = true;
};
