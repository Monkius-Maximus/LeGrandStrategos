#pragma once

#include "CoreMinimal.h"
#include "Diplomacy/DiplomaticStatus.h"
#include "DiplomaticRelation.generated.h"

/**
 * FNationPair — chave canônica para uma relação bilateral.
 *
 * A ordem de A/B é normalizada lexicograficamente em Make() para que
 * (Albion, Galia) e (Galia, Albion) gerem a mesma chave e o mesmo hash.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FNationPair
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere) FName A;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere) FName B;

	FNationPair() = default;
	FNationPair(FName InA, FName InB) : A(InA), B(InB) {}

	static FNationPair Make(FName X, FName Y)
	{
		return (X.Compare(Y) <= 0) ? FNationPair(X, Y) : FNationPair(Y, X);
	}

	bool IsValid() const { return !A.IsNone() && !B.IsNone() && A != B; }

	bool operator==(const FNationPair& Other) const
	{
		return A == Other.A && B == Other.B;
	}

	friend uint32 GetTypeHash(const FNationPair& P)
	{
		return HashCombine(GetTypeHash(P.A), GetTypeHash(P.B));
	}
};

/**
 * FDiplomaticRelation — estado runtime de uma relação bilateral.
 *
 * V1: status + opinion + data da última mudança de status. Trust separado,
 * tratados ativos, OpinionBreakdown e CB ficam em iterações posteriores.
 */
USTRUCT(BlueprintType)
struct STRATEGOSCORE_API FDiplomaticRelation
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, VisibleAnywhere) FNationPair Pair;
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere) EDiplomaticStatus Status = EDiplomaticStatus::Peace;

	/** -100..+100. Opinião agregada para usos de IA e UI. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere) float Opinion = 0.f;

	/** Data da última transição de status. Usada para tooltips e cooldowns futuros. */
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere) FDateTime LastStatusChange;
};
