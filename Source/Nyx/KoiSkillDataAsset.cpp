#include "KoiSkillDataAsset.h"

FName UKoiSkillDataAsset::GetStableSkillId() const
{
	return SkillId.IsNone() ? GetFName() : SkillId;
}

int32 UKoiSkillDataAsset::GetKoiCostForRank(int32 CurrentRank) const
{
	return FMath::Max(0, BaseKoiCost + FMath::Max(0, CurrentRank) * KoiCostIncreasePerRank);
}
