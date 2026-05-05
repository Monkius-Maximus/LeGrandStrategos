#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Nation.generated.h"

/**
 * UNation — entidade política. No MVP detém id, nome, cor e províncias.
 * Economia, política interna, diplomacia, IA chegam nas etapas seguintes.
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UNation : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Nation")
	FName Id;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Nation")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Nation")
	FLinearColor Color = FLinearColor::White;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Nation")
	FName CapitalProvinceId;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Nation")
	TArray<FName> OwnedProvinceIds;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Nation")
	bool bIsPlayerControlled = false;
};
