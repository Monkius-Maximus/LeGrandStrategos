#pragma once

#include "CoreMinimal.h"
#include "NationalStockpile.generated.h"

/**
 * FNationalStockpile — Pool agregado de bens da nação.
 *
 * v1: tudo nacional (sem mercados regionais). Bens produzidos vão para
 * Stocks; consumo (POPs + indústria) sai dali. Demand/Supply são limpos
 * a cada início de tick e populados durante as fases de produção e
 * consumo — usados para preço dinâmico e índices estratégicos.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FNationalStockpile
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category = "Stockpile")
	TMap<FName, float> Stocks;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Stockpile|Last Month")
	TMap<FName, float> Demand;

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere, Category = "Stockpile|Last Month")
	TMap<FName, float> Supply;

	float GetStock(FName GoodId) const
	{
		const float* V = Stocks.Find(GoodId);
		return V ? *V : 0.f;
	}

	void AddStock(FName GoodId, float Amount)
	{
		float& Cur = Stocks.FindOrAdd(GoodId, 0.f);
		Cur = FMath::Max(0.f, Cur + Amount);
	}

	bool TryConsume(FName GoodId, float Amount)
	{
		float& Cur = Stocks.FindOrAdd(GoodId, 0.f);
		if (Cur < Amount)
		{
			return false;
		}
		Cur -= Amount;
		return true;
	}

	float ConsumeUpTo(FName GoodId, float WantedAmount)
	{
		float& Cur = Stocks.FindOrAdd(GoodId, 0.f);
		const float Taken = FMath::Min(Cur, WantedAmount);
		Cur -= Taken;
		return Taken;
	}

	void RecordDemand(FName GoodId, float Amount)
	{
		float& V = Demand.FindOrAdd(GoodId, 0.f);
		V += Amount;
	}

	void RecordSupply(FName GoodId, float Amount)
	{
		float& V = Supply.FindOrAdd(GoodId, 0.f);
		V += Amount;
	}

	void ResetMonthlyCounters()
	{
		for (auto& Pair : Demand) { Pair.Value = 0.f; }
		for (auto& Pair : Supply) { Pair.Value = 0.f; }
	}

	/**
	 * Razão Supply/Demand clampada [0..2]. Usada para preço dinâmico e
	 * índices estratégicos. Demand=0 → 1.0 (neutro).
	 */
	float GetSupplyRatio(FName GoodId) const
	{
		const float* D = Demand.Find(GoodId);
		const float* S = Supply.Find(GoodId);
		const float Dv = D ? *D : 0.f;
		const float Sv = S ? *S : 0.f;
		if (Dv <= KINDA_SMALL_NUMBER) return 1.0f;
		return FMath::Clamp(Sv / Dv, 0.f, 2.f);
	}
};
