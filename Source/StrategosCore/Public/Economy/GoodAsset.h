#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GoodAsset.generated.h"

class UTexture2D;

/**
 * EGoodCategory — Classificação que a UI e o consumption basket usam.
 *
 * Raw         : recursos extraídos (Tier 0)
 * Staple      : bens de subsistência (Bread, basic Cloth)
 * Industrial  : insumos de produção (Iron, Lumber, Tools)
 * Luxury      : conforto / consumo elevado (Garments, Furniture, Wine futuro)
 */
UENUM(BlueprintType)
enum class EGoodCategory : uint8
{
	Raw,
	Staple,
	Industrial,
	Luxury
};

/**
 * UGoodAsset — Definição de um bem econômico (DataAsset).
 *
 * Bens são extensíveis: novos tipos viram novos UGoodAsset sem tocar C++.
 * O subsistema descobre os bens existentes via UEconomyContentRegistry
 * (commit 5).
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UGoodAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Good")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Good")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Good")
	TSoftObjectPtr<UTexture2D> Icon;

	/** 0 = raw, 1 = processed, 2 = consumer/light industrial, 3 = heavy industrial */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Good", meta = (ClampMin = "0", ClampMax = "5"))
	int32 Tier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Good", meta = (ClampMin = "0.01"))
	float BasePrice = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Good")
	EGoodCategory Category = EGoodCategory::Raw;
};
