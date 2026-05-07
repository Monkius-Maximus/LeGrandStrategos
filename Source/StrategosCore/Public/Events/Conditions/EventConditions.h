#pragma once

#include "CoreMinimal.h"
#include "Events/EventCondition.h"
#include "Economy/PopStratum.h"
#include "EventConditions.generated.h"

/**
 * Condição: Treasury da nação fonte (ou explícita) está abaixo de um limite.
 */
UCLASS(meta = (DisplayName = "Treasury Below"))
class STRATEGOSCORE_API UCondition_TreasuryBelow : public UEventCondition
{
	GENERATED_BODY()

public:
	/** Se vazio, usa Context.SourceNationId. */
	UPROPERTY(EditAnywhere, Category = "Condition")
	FName NationId;

	UPROPERTY(EditAnywhere, Category = "Condition")
	float Threshold = 0.f;

	virtual bool Evaluate_Implementation(UWorldState* WorldState, const FEventContext& Context) override;
};

/**
 * Condição: existe algum POP do estrato dado com Loyalty abaixo do limite,
 * em qualquer província da nação fonte.
 */
UCLASS(meta = (DisplayName = "Loyalty Below"))
class STRATEGOSCORE_API UCondition_LoyaltyBelow : public UEventCondition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Condition")
	FName NationId;

	UPROPERTY(EditAnywhere, Category = "Condition")
	EPopStratum Stratum = EPopStratum::FactoryWorker;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float Threshold = 0.6f;

	virtual bool Evaluate_Implementation(UWorldState* WorldState, const FEventContext& Context) override;
};

/**
 * Condição: stockpile da nação tem ao menos MinAmount do bem GoodId.
 */
UCLASS(meta = (DisplayName = "Has Good In Stockpile"))
class STRATEGOSCORE_API UCondition_HasGoodInStockpile : public UEventCondition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Condition")
	FName NationId;

	UPROPERTY(EditAnywhere, Category = "Condition")
	FName GoodId;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = "0.0"))
	float MinAmount = 0.f;

	virtual bool Evaluate_Implementation(UWorldState* WorldState, const FEventContext& Context) override;
};
