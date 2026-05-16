#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Diplomacy/DiplomaticRelation.h"
#include "DiplomacySubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDiplomaticRelationChanged, const FDiplomaticRelation&, Relation);

/**
 * UDiplomacySubsystem — registro central de relações bilaterais entre nações.
 *
 * V1 (data layer):
 *  - Matriz esparsa de relações via TMap<FNationPair, FDiplomaticRelation>
 *  - Getters/setters básicos para Status e Opinion
 *  - Convenience queries (AreAtWar, AreAllied)
 *  - Delegate único de mudança para a UI/AI observarem
 *
 * Ações (DeclareWar, ProposeAlliance, OfferGift) e tratados entram em
 * iterações seguintes; elas vão chamar SetStatus/AdjustOpinion daqui.
 */
UCLASS()
class STRATEGOSCORE_API UDiplomacySubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;
	virtual bool DoesSupportWorldType(EWorldType::Type WorldType) const override;

	/** Cria ou substitui a relação entre A e B com status/opinion dados. */
	UFUNCTION(BlueprintCallable, Category = "Strategos|Diplomacy")
	void SetRelation(FName A, FName B, EDiplomaticStatus Status, float Opinion);

	UFUNCTION(BlueprintCallable, Category = "Strategos|Diplomacy")
	void SetStatus(FName A, FName B, EDiplomaticStatus NewStatus);

	UFUNCTION(BlueprintCallable, Category = "Strategos|Diplomacy")
	void SetOpinion(FName A, FName B, float NewOpinion);

	UFUNCTION(BlueprintCallable, Category = "Strategos|Diplomacy")
	void AdjustOpinion(FName A, FName B, float Delta);

	UFUNCTION(BlueprintPure, Category = "Strategos|Diplomacy")
	bool HasRelation(FName A, FName B) const;

	UFUNCTION(BlueprintPure, Category = "Strategos|Diplomacy")
	FDiplomaticRelation GetRelation(FName A, FName B) const;

	UFUNCTION(BlueprintPure, Category = "Strategos|Diplomacy")
	EDiplomaticStatus GetStatus(FName A, FName B) const;

	UFUNCTION(BlueprintPure, Category = "Strategos|Diplomacy")
	float GetOpinion(FName A, FName B) const;

	UFUNCTION(BlueprintPure, Category = "Strategos|Diplomacy")
	bool AreAtWar(FName A, FName B) const;

	UFUNCTION(BlueprintPure, Category = "Strategos|Diplomacy")
	bool AreAllied(FName A, FName B) const;

	/** Todas as nações com as quais Nation tem alguma relação registrada. */
	UFUNCTION(BlueprintPure, Category = "Strategos|Diplomacy")
	TArray<FName> GetKnownCounterparts(FName Nation) const;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Diplomacy")
	FOnDiplomaticRelationChanged OnRelationChanged;

	// --- Save/Load support -------------------------------------------------

	const TMap<FNationPair, FDiplomaticRelation>& GetRelationsRaw() const { return Relations; }

	void RestoreRelations(const TArray<FDiplomaticRelation>& Snapshot);

private:
	void BroadcastChange(const FDiplomaticRelation& Rel);

	UPROPERTY()
	TMap<FNationPair, FDiplomaticRelation> Relations;
};
