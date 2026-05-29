#include "BattleAIProfile.h"

float UBattleAIProfile::GetCategoryBias(ECardCategory Cat) const
{
	const float* Found = CategoryBias.Find(Cat);
	return Found ? *Found : 1.0f;
}
