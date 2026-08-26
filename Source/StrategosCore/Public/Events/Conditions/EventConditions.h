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

// ---------------------------------------------------------------------------
// Composição lógica.
//
// UEventAsset::Conditions e FEventChoice::AvailabilityConditions já são um AND
// implícito. Estas três permitem expressar OR e NOT, e aninhar árvores
// arbitrárias sem escrever C++ novo para cada combinação.

/** Verdadeira quando TODAS as sub-condições passam. Lista vazia = verdadeira. */
UCLASS(meta = (DisplayName = "AND (todas)"))
class STRATEGOSCORE_API UCondition_And : public UEventCondition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "Condition")
	TArray<TObjectPtr<UEventCondition>> SubConditions;

	virtual bool Evaluate_Implementation(UWorldState* WorldState, const FEventContext& Context) override;
};

/** Verdadeira quando ao menos UMA sub-condição passa. Lista vazia = falsa. */
UCLASS(meta = (DisplayName = "OR (qualquer)"))
class STRATEGOSCORE_API UCondition_Or : public UEventCondition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "Condition")
	TArray<TObjectPtr<UEventCondition>> SubConditions;

	virtual bool Evaluate_Implementation(UWorldState* WorldState, const FEventContext& Context) override;
};

/** Inverte a sub-condição. Sem sub-condição setada, retorna falsa. */
UCLASS(meta = (DisplayName = "NOT (inverte)"))
class STRATEGOSCORE_API UCondition_Not : public UEventCondition
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Instanced, Category = "Condition")
	TObjectPtr<UEventCondition> SubCondition;

	virtual bool Evaluate_Implementation(UWorldState* WorldState, const FEventContext& Context) override;
};

// ---------------------------------------------------------------------------
// Consulta ao histórico.

/** Verdadeira se o evento dado já disparou para a nação em qualquer momento. */
UCLASS(meta = (DisplayName = "Evento Ja Disparou"))
class STRATEGOSCORE_API UCondition_PreviousEventFired : public UEventCondition
{
	GENERATED_BODY()

public:
	/** Se vazio, usa Context.SourceNationId. */
	UPROPERTY(EditAnywhere, Category = "Condition")
	FName NationId;

	UPROPERTY(EditAnywhere, Category = "Condition")
	FName EventId;

	/** Inverte o teste: verdadeira quando o evento NUNCA disparou. */
	UPROPERTY(EditAnywhere, Category = "Condition")
	bool bInvert = false;

	virtual bool Evaluate_Implementation(UWorldState* WorldState, const FEventContext& Context) override;
};

/**
 * Verdadeira se a escolha mais recente registrada para o evento dado bate com
 * ChoiceIndex. Consulta o log de histórico, que tem teto de tamanho: um evento
 * muito antigo pode já ter saído do log e retornar falso.
 */
UCLASS(meta = (DisplayName = "Escolha Anterior Foi"))
class STRATEGOSCORE_API UCondition_PreviousChoiceWas : public UEventCondition
{
	GENERATED_BODY()

public:
	/** Se vazio, usa Context.SourceNationId. */
	UPROPERTY(EditAnywhere, Category = "Condition")
	FName NationId;

	UPROPERTY(EditAnywhere, Category = "Condition")
	FName EventId;

	UPROPERTY(EditAnywhere, Category = "Condition", meta = (ClampMin = "0"))
	int32 ChoiceIndex = 0;

	virtual bool Evaluate_Implementation(UWorldState* WorldState, const FEventContext& Context) override;
};
