#pragma once

#include "CoreMinimal.h"
#include "Events/EventEffect.h"
#include "Economy/PopStratum.h"
#include "Economy/GoodAmount.h"
#include "EventEffects.generated.h"

/** Adiciona Amount ao Treasury.Balance da nação fonte (ou explícita). */
UCLASS(meta = (DisplayName = "Add Gold"))
class STRATEGOSCORE_API UEffect_AddGold : public UEventEffect
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Effect")
	FName NationId;

	UPROPERTY(EditAnywhere, Category = "Effect")
	float Amount = 0.f;

	virtual void Apply_Implementation(UWorldState* WorldState, const FEventContext& Context) override;
	virtual FText GetDescription_Implementation() const override;
};

/** Soma Delta na Loyalty de todos POPs do estrato em todas províncias da nação. */
UCLASS(meta = (DisplayName = "Add Pop Loyalty"))
class STRATEGOSCORE_API UEffect_AddPopLoyalty : public UEventEffect
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Effect")
	FName NationId;

	UPROPERTY(EditAnywhere, Category = "Effect")
	EPopStratum Stratum = EPopStratum::FactoryWorker;

	UPROPERTY(EditAnywhere, Category = "Effect", meta = (ClampMin = "-1.0", ClampMax = "1.0"))
	float Delta = 0.05f;

	/** Se true, aplica em TODOS estratos (ignora Stratum). */
	UPROPERTY(EditAnywhere, Category = "Effect")
	bool bAllStrata = false;

	virtual void Apply_Implementation(UWorldState* WorldState, const FEventContext& Context) override;
	virtual FText GetDescription_Implementation() const override;
};

/** Adiciona/remove uma lista de bens ao Stockpile da nação. */
UCLASS(meta = (DisplayName = "Add Goods To Stockpile"))
class STRATEGOSCORE_API UEffect_AddGoodsToStockpile : public UEventEffect
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Effect")
	FName NationId;

	/** Use Amount negativo para drenar (ex.: bandit raid). */
	UPROPERTY(EditAnywhere, Category = "Effect")
	TArray<FGoodAmount> Goods;

	virtual void Apply_Implementation(UWorldState* WorldState, const FEventContext& Context) override;
	virtual FText GetDescription_Implementation() const override;
};

/** Encadeia outro evento — permite narrativas multi-passo via DataAssets. */
UCLASS(meta = (DisplayName = "Fire Event"))
class STRATEGOSCORE_API UEffect_FireEvent : public UEventEffect
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Effect")
	FName EventId;

	UPROPERTY(EditAnywhere, Category = "Effect")
	FName TargetNationId;

	virtual void Apply_Implementation(UWorldState* WorldState, const FEventContext& Context) override;
	virtual FText GetDescription_Implementation() const override;
};
