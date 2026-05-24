#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "TradeHub.generated.h"

/**
 * UTradeHub — Nó de comércio regional onde a infraestrutura converge
 * (doc §5).
 *
 * Runtime UObject (não DataAsset): hubs são criados dinamicamente quando
 * a banda agregada de um cluster de províncias passa um threshold. A
 * province "host" é onde o hub fisicamente reside; províncias membros
 * roteiam produção via o hub para alcançar o Mercado Global.
 *
 * Toda interação com o hub é abstrata matematicamente — não há entidade
 * física no mapa para comércio civil (ver doc §5, "Abstração de
 * Performance").
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UTradeHub : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Hub")
	FName Id;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Hub")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Hub")
	FName HostProvinceId;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Hub")
	FName OwnerNationId;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Hub")
	TArray<FName> MemberProvinceIds;

	/** Banda agregada que o hub redistribui ao mercado global. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Hub|Capacity", meta = (ClampMin = "0.0"))
	float ThroughputPerMonth = 0.f;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Hub|Capacity", meta = (ClampMin = "0.0"))
	float UsedThroughputThisTick = 0.f;

	/** Taxa cobrada pelo hub por unidade roteada (vai pro tesouro do dono). */
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Hub|Economy", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float TariffRate = 0.05f;
};
