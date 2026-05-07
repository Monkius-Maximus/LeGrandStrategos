#pragma once

#include "CoreMinimal.h"
#include "GoodAmount.generated.h"

/**
 * FGoodAmount — Par (GoodId, Amount) usado em recipes, custos, snapshots.
 *
 * Convencionalmente Amount é positivo (consumo ou produção é decidido pelo
 * contexto onde a struct aparece).
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FGoodAmount
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Good")
	FName GoodId;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Good", meta = (ClampMin = "0"))
	float Amount = 0.f;

	FGoodAmount() = default;
	FGoodAmount(FName InId, float InAmt) : GoodId(InId), Amount(InAmt) {}
};
