#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Army.generated.h"

/**
 * UArmy — unidade estratégica que ocupa províncias e se move entre elas.
 *
 * Stage 1 (MVP): dono, província atual, ordem de movimento (target +
 * dias restantes) e contagem de soldados. Composição detalhada (regimentos,
 * comandantes, equipamento) entra na Etapa 3 junto com o sistema de unidades.
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UArmy : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Army")
	FName Id;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Army")
	FText DisplayName;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Army")
	FName OwnerNationId;

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Army")
	FName CurrentProvinceId;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Army")
	FName MoveTargetProvinceId;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Army")
	int32 MoveDaysRemaining = 0;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Army")
	int32 ManpowerCount = 1000;

	UFUNCTION(BlueprintPure, Category = "Army")
	bool IsMoving() const { return !MoveTargetProvinceId.IsNone() && MoveDaysRemaining > 0; }
};
