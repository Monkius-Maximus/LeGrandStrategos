#include "BattleTypes.h"
#include "StrategosBattle.h"

// ─────────────────────────────────────────────────────────────────────────────
// FBattleSide
// ─────────────────────────────────────────────────────────────────────────────

int32 FBattleSide::TotalCurrentStrength() const
{
	int32 Total = 0;
	for (const FRegimentBattleState& Reg : Regiments)
	{
		Total += Reg.CurrentStrength;
	}
	return Total;
}

int32 FBattleSide::TotalInitialStrength() const
{
	int32 Total = 0;
	for (const FRegimentBattleState& Reg : Regiments)
	{
		Total += Reg.InitialStrength;
	}
	return Total;
}

float FBattleSide::StrengthRatio() const
{
	const int32 Init = TotalInitialStrength();
	return Init > 0 ? static_cast<float>(TotalCurrentStrength()) / Init : 0.f;
}

float FBattleSide::ComputeFightingPower() const
{
	float Power = 0.f;
	for (const FRegimentBattleState& Reg : Regiments)
	{
		if (!Reg.IsActive()) continue;

		float StanceMod = 1.0f;
		switch (Reg.Stance)
		{
		case EBattleStance::Aggressive: StanceMod = 1.2f; break;
		case EBattleStance::Hold:       StanceMod = 1.0f; break;
		case EBattleStance::Skirmish:   StanceMod = 0.8f; break;
		case EBattleStance::Reserve:    StanceMod = 0.5f; break;
		}

		Power += Reg.CurrentStrength * (Reg.ATQ / 100.f) * StanceMod;
	}
	return Power;
}

float FBattleSide::ComputeAverageDEF() const
{
	int32 TotalStr = 0;
	float WeightedDEF = 0.f;
	for (const FRegimentBattleState& Reg : Regiments)
	{
		if (!Reg.IsActive()) continue;
		TotalStr += Reg.CurrentStrength;
		WeightedDEF += Reg.CurrentStrength * Reg.DEF;
	}
	return TotalStr > 0 ? WeightedDEF / TotalStr : 50.f;
}

float FBattleSide::ComputeAverageMOR() const
{
	int32 TotalStr = 0;
	float WeightedMOR = 0.f;
	for (const FRegimentBattleState& Reg : Regiments)
	{
		if (!Reg.IsActive()) continue;
		TotalStr += Reg.CurrentStrength;
		WeightedMOR += Reg.CurrentStrength * Reg.MOR;
	}
	return TotalStr > 0 ? WeightedMOR / TotalStr : 50.f;
}

bool FBattleSide::HasRouted() const
{
	return bRouted;
}

bool FBattleSide::IsDefeated() const
{
	return bRouted || TotalCurrentStrength() == 0;
}

// ─────────────────────────────────────────────────────────────────────────────
// FBattleContext
// ─────────────────────────────────────────────────────────────────────────────

bool FBattleContext::IsBattleOver() const
{
	return Attacker.IsDefeated()
		|| Defender.IsDefeated()
		|| CurrentPhase == EBattlePhase::Resolved;
}
