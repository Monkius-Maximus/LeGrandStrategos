#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Economy/PopStratum.h"
#include "Economy/GoodAmount.h"
#include "ProductionMethodAsset.generated.h"

class UTexture2D;

USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FStratumEmployment
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Employment")
	EPopStratum Stratum = EPopStratum::Laborer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Employment", meta = (ClampMin = "0"))
	int32 Headcount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Employment", meta = (ClampMin = "0.0"))
	float WagePerWorker = 1.0f;
};

/**
 * UProductionMethodAsset — A "carta" da economia.
 *
 * Define receita: inputs/outputs por slot (1 nível de prédio = 1 slot
 * neste contrato), emprego necessário por estrato + salário base, e
 * eventual gate de tecnologia (None = livre).
 *
 * Tier dita a ordem de execução no tick de produção: Tier 0 (raws)
 * primeiro, depois Tier 1, etc. Receitas só consomem bens de tier <=
 * o seu próprio (DAG sem ciclos).
 */
UCLASS(BlueprintType)
class STRATEGOSCORE_API UProductionMethodAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Method")
	FName Id;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Method")
	FText DisplayName;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Method")
	TSoftObjectPtr<UTexture2D> Icon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Method", meta = (ClampMin = "0", ClampMax = "5"))
	int32 ProductionTier = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Method|Recipe")
	TArray<FGoodAmount> InputsPerSlot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Method|Recipe")
	TArray<FGoodAmount> OutputsPerSlot;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Method|Labor")
	TArray<FStratumEmployment> EmploymentPerSlot;

	/** None = sem gate; senão, requer essa tech estar pesquisada (Etapa 3). */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Method|Gating")
	FName RequiredTechId;

	/**
	 * Se true, esta PM extrai bens dos InputsPerSlot que NÃO são consumidos —
	 * representam recursos naturais. O EconomySubsystem multiplica o Output
	 * pelo RawResourcePotential[GoodId] da província (mines/farms).
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Method|Recipe")
	bool bRequiresRawResource = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Method|Recipe")
	FName RawResourceGoodId;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Method|Cost", meta = (ClampMin = "0.0"))
	float MaintenancePerSlot = 1.0f;
};
