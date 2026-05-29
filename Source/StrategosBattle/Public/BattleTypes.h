#pragma once

#include "CoreMinimal.h"
#include "BattleTypes.generated.h"

// ─────────────────────────────────────────────────────────────────────────────
// Enums
// ─────────────────────────────────────────────────────────────────────────────

UENUM(BlueprintType)
enum class EBattleType : uint8
{
	DirectClash    UMETA(DisplayName = "Confronto direto"),
	BorderSkirmish UMETA(DisplayName = "Escaramuça de fronteira"),
	Raid           UMETA(DisplayName = "Incursão"),
	Siege          UMETA(DisplayName = "Cerco"),
	RangedHarass   UMETA(DisplayName = "Assédio à distância")
};

UENUM(BlueprintType)
enum class EBattleTerrain : uint8
{
	Plains   UMETA(DisplayName = "Planícies"),
	Forest   UMETA(DisplayName = "Floresta"),
	Hills    UMETA(DisplayName = "Colinas"),
	Mountain UMETA(DisplayName = "Montanha"),
	River    UMETA(DisplayName = "Rio"),
	Urban    UMETA(DisplayName = "Urbano"),
	Coast    UMETA(DisplayName = "Costa")
};

UENUM(BlueprintType)
enum class EBattleWeather : uint8
{
	Clear UMETA(DisplayName = "Limpo"),
	Rain  UMETA(DisplayName = "Chuva"),
	Storm UMETA(DisplayName = "Tempestade"),
	Fog   UMETA(DisplayName = "Névoa"),
	Snow  UMETA(DisplayName = "Neve")
};

UENUM(BlueprintType)
enum class EBattlePhase : uint8
{
	Setup      UMETA(DisplayName = "Posicionamento"),
	Engagement UMETA(DisplayName = "Engajamento"),
	Climax     UMETA(DisplayName = "Clímax"),
	Pursuit    UMETA(DisplayName = "Perseguição"),
	Resolved   UMETA(DisplayName = "Resolvida")
};

UENUM(BlueprintType)
enum class EBattlePosition : uint8
{
	Frontline  UMETA(DisplayName = "Linha de frente"),
	Flank      UMETA(DisplayName = "Flanco"),
	Rear       UMETA(DisplayName = "Retaguarda"),
	HighGround UMETA(DisplayName = "Terreno elevado"),
	Crossing   UMETA(DisplayName = "Travessia")
};

UENUM(BlueprintType)
enum class EBattleOutcome : uint8
{
	AttackerVictory UMETA(DisplayName = "Vitória do atacante"),
	DefenderVictory UMETA(DisplayName = "Vitória do defensor"),
	Stalemate       UMETA(DisplayName = "Empate")
};

UENUM(BlueprintType)
enum class EBattleLogType : uint8
{
	PhaseChanged   UMETA(DisplayName = "Fase mudou"),
	DamageDealt    UMETA(DisplayName = "Dano causado"),
	MoraleChanged  UMETA(DisplayName = "Moral alterada"),
	SideRouted     UMETA(DisplayName = "Lado em fuga"),
	BattleResolved UMETA(DisplayName = "Batalha resolvida")
};

UENUM(BlueprintType)
enum class ERegimentType : uint8
{
	Infantry  UMETA(DisplayName = "Infantaria"),
	Cavalry   UMETA(DisplayName = "Cavalaria"),
	Archer    UMETA(DisplayName = "Arqueiros"),
	Pike      UMETA(DisplayName = "Piqueiros"),
	Artillery UMETA(DisplayName = "Artilharia")
};

UENUM(BlueprintType)
enum class EBattleStance : uint8
{
	Aggressive UMETA(DisplayName = "Agressivo"),
	Hold       UMETA(DisplayName = "Segurar"),
	Skirmish   UMETA(DisplayName = "Escaramuçar"),
	Reserve    UMETA(DisplayName = "Reserva")
};

// ─────────────────────────────────────────────────────────────────────────────
// Structs de log e efeitos ativos
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct STRATEGOSBATTLE_API FBattleLogEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) int32 Round = 0;
	UPROPERTY(BlueprintReadOnly) EBattlePhase Phase = EBattlePhase::Setup;
	UPROPERTY(BlueprintReadOnly) int32 ActorSideIndex = 0;
	UPROPERTY(BlueprintReadOnly) EBattleLogType Type = EBattleLogType::DamageDealt;
	UPROPERTY(BlueprintReadOnly) FString Description;
	UPROPERTY(BlueprintReadOnly) int32 NumericValue = 0;
	UPROPERTY(BlueprintReadOnly) FGuid TargetId;
};

USTRUCT(BlueprintType)
struct STRATEGOSBATTLE_API FActiveBattleEffect
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FName EffectId;
	UPROPERTY(BlueprintReadOnly) FText Label;
	UPROPERTY(BlueprintReadOnly) float Value = 0.f;
	UPROPERTY(BlueprintReadOnly) int32 RoundsRemaining = 1;
	UPROPERTY(BlueprintReadOnly) bool bPositive = true;
};

// ─────────────────────────────────────────────────────────────────────────────
// Regimento dentro da batalha (cópia do estado estratégico — não referência)
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct STRATEGOSBATTLE_API FRegimentBattleState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FGuid RegimentId;
	UPROPERTY(BlueprintReadOnly) FName SourceArmyId;
	UPROPERTY(BlueprintReadOnly) ERegimentType Type = ERegimentType::Infantry;
	UPROPERTY(BlueprintReadOnly) int32 InitialStrength = 1000;
	UPROPERTY(BlueprintReadWrite) int32 CurrentStrength = 1000;
	UPROPERTY(BlueprintReadWrite) float Morale = 100.f;
	UPROPERTY(BlueprintReadWrite) float OrganizationLeft = 1.f;
	UPROPERTY(BlueprintReadWrite) EBattleStance Stance = EBattleStance::Hold;

	// Stats capturados no momento do engajamento (imutáveis durante a batalha)
	UPROPERTY(BlueprintReadOnly) int32 ATQ = 50;
	UPROPERTY(BlueprintReadOnly) int32 DEF = 50;
	UPROPERTY(BlueprintReadOnly) int32 MOR = 50;

	bool IsActive() const { return CurrentStrength > 0 && OrganizationLeft > 0.f; }
};

// ─────────────────────────────────────────────────────────────────────────────
// Resultado intermediário do CombatTick (para log e debug)
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT()
struct STRATEGOSBATTLE_API FCombatTickResult
{
	GENERATED_BODY()

	float TotalDamage = 0.f;
	float TerrainMod  = 1.f;
	float WeatherMod  = 1.f;
	float MoraleMod   = 1.f;
	float SupplyMod   = 1.f;
	float PositionMod = 1.f;
};

// ─────────────────────────────────────────────────────────────────────────────
// Um dos lados da batalha
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct STRATEGOSBATTLE_API FBattleSide
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) int32 NationId = -1;
	UPROPERTY(BlueprintReadOnly) FName CommanderId;
	UPROPERTY(BlueprintReadOnly) TArray<FRegimentBattleState> Regiments;

	UPROPERTY(BlueprintReadWrite) int32 CommandPoints = 3;
	UPROPERTY(BlueprintReadOnly) int32 MaxCommandPoints = 3;
	UPROPERTY(BlueprintReadWrite) float Morale = 100.f;
	UPROPERTY(BlueprintReadWrite) float Supply = 1.f;
	UPROPERTY(BlueprintReadWrite) float Cohesion = 1.f;

	UPROPERTY(BlueprintReadWrite) EBattlePosition Position = EBattlePosition::Frontline;
	UPROPERTY(BlueprintReadOnly) bool bHasInitiative = false;
	UPROPERTY(BlueprintReadOnly) bool bRouted = false;

	// Stage 4: deck de cartas (UBattleCardAsset*)
	// TArray<UBattleCardAsset*> DrawPile;
	// TArray<UBattleCardAsset*> Hand;
	// TArray<UBattleCardAsset*> DiscardPile;
	// TArray<UBattleCardAsset*> ExhaustPile;

	// Implementados em BattleTypes.cpp
	int32 TotalCurrentStrength() const;
	int32 TotalInitialStrength() const;
	float StrengthRatio() const;
	float ComputeFightingPower() const;
	float ComputeAverageDEF() const;
	float ComputeAverageMOR() const;
	bool  HasRouted() const;
	bool  IsDefeated() const;
};

// ─────────────────────────────────────────────────────────────────────────────
// Contexto vivo da batalha — DTO central
// ─────────────────────────────────────────────────────────────────────────────

USTRUCT(BlueprintType)
struct STRATEGOSBATTLE_API FBattleContext
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly) FGuid BattleId;
	UPROPERTY(BlueprintReadOnly) EBattleType Type = EBattleType::DirectClash;

	UPROPERTY(BlueprintReadWrite) FBattleSide Attacker;
	UPROPERTY(BlueprintReadWrite) FBattleSide Defender;

	UPROPERTY(BlueprintReadOnly) EBattleTerrain Terrain = EBattleTerrain::Plains;
	UPROPERTY(BlueprintReadOnly) EBattleWeather Weather = EBattleWeather::Clear;
	UPROPERTY(BlueprintReadOnly) int32 ProvinceId = -1;

	UPROPERTY(BlueprintReadOnly) EBattlePhase CurrentPhase = EBattlePhase::Setup;
	UPROPERTY(BlueprintReadOnly) int32 CurrentRound = 0;
	UPROPERTY(BlueprintReadOnly) int32 TotalRounds = 0;

	UPROPERTY(BlueprintReadOnly) TArray<FActiveBattleEffect> AttackerEffects;
	UPROPERTY(BlueprintReadOnly) TArray<FActiveBattleEffect> DefenderEffects;

	UPROPERTY(BlueprintReadOnly) TArray<FBattleLogEntry> Log;

	int32 Seed = 0;

	// Implementado em BattleTypes.cpp
	bool IsBattleOver() const;
};
