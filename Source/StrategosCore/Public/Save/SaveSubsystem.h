#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "SaveSubsystem.generated.h"

class UStrategosSaveData;
class UWorldState;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSaveCompleted, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLoadCompleted, bool, bSuccess);

/**
 * USaveSubsystem (v1) — Carrega/salva o estado do mundo em slots nomeados.
 *
 * Stage 1 (MVP): serializa UWorldState como arrays de records flat
 * (FNationRecord/FProvinceRecord/FArmyRecord) num UStrategosSaveData,
 * depois usa UGameplayStatics::SaveGameToSlot. Reload reverte o processo
 * e recria os UObjects via WorldState->AddNation/Province/Army.
 *
 * Versionamento e migração entram quando estado complexo (Brains de IA,
 * Modifiers, Treaties) precisar evoluir compatível com saves antigos.
 */
UCLASS()
class STRATEGOSCORE_API USaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Strategos|Save")
	bool SaveToSlot(const FString& SlotName);

	UFUNCTION(BlueprintCallable, Category = "Strategos|Save")
	bool LoadFromSlot(const FString& SlotName);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Strategos|Save")
	bool DoesSaveExist(const FString& SlotName) const;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Save")
	FOnSaveCompleted OnSaveCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Strategos|Save")
	FOnLoadCompleted OnLoadCompleted;

private:
	UStrategosSaveData* CaptureSnapshot() const;
	bool ApplySnapshot(const UStrategosSaveData& Snapshot);

	UWorldState* ResolveWorldState() const;
};
