#include "KoiSkillTreeComponent.h"

#include "EconomyComponent.h"
#include "FishingComponent.h"
#include "Starwell.h"

UKoiSkillTreeComponent::UKoiSkillTreeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

bool UKoiSkillTreeComponent::InvestSkill(UKoiSkillDataAsset* Skill, UEconomyComponent* EconomyComponent)
{
	if (!CanInvestSkill(Skill, EconomyComponent))
	{
		return false;
	}

	const int32 KoiCost = GetNextRankKoiCost(Skill);
	if (EconomyComponent != nullptr && !EconomyComponent->SpendResource(ENyxResourceType::Koi, KoiCost))
	{
		return false;
	}

	const FName SkillSaveKey = GetSkillSaveKey(Skill);
	int32& Rank = InvestedSkillRanks.FindOrAdd(SkillSaveKey);
	++Rank;

	OnKoiSkillRankChanged.Broadcast(this, Skill, Rank, KoiCost);
	return true;
}

bool UKoiSkillTreeComponent::CanInvestSkill(const UKoiSkillDataAsset* Skill, const UEconomyComponent* EconomyComponent) const
{
	if (Skill == nullptr || EconomyComponent == nullptr)
	{
		return false;
	}

	const FName SkillSaveKey = GetSkillSaveKey(Skill);
	if (SkillSaveKey.IsNone())
	{
		return false;
	}

	const int32 CurrentRank = GetSkillRank(Skill);
	if (CurrentRank >= Skill->MaxRank)
	{
		return false;
	}

	return PrerequisitesMet(Skill) && EconomyComponent->GetResourceAmount(ENyxResourceType::Koi) >= GetNextRankKoiCost(Skill);
}

int32 UKoiSkillTreeComponent::GetSkillRank(const UKoiSkillDataAsset* Skill) const
{
	return GetSkillRankById(GetSkillSaveKey(Skill));
}

int32 UKoiSkillTreeComponent::GetSkillRankById(FName SkillId) const
{
	if (SkillId.IsNone())
	{
		return 0;
	}

	if (const int32* Rank = InvestedSkillRanks.Find(SkillId))
	{
		return FMath::Max(0, *Rank);
	}

	return 0;
}

int32 UKoiSkillTreeComponent::GetNextRankKoiCost(const UKoiSkillDataAsset* Skill) const
{
	return Skill != nullptr ? Skill->GetKoiCostForRank(GetSkillRank(Skill)) : 0;
}

float UKoiSkillTreeComponent::GetPlayerPowerMultiplier() const
{
	return FMath::Max(1.0f, 1.0f + SumEffectValue(ENyxKoiSkillEffect::PlayerPower));
}

float UKoiSkillTreeComponent::GetBiteTimeMultiplier() const
{
	return FMath::Clamp(1.0f - SumEffectValue(ENyxKoiSkillEffect::FasterBites), 0.1f, 1.0f);
}

int32 UKoiSkillTreeComponent::GetBonusFishPulls() const
{
	return FMath::Max(0, FMath::FloorToInt(SumEffectValue(ENyxKoiSkillEffect::BonusFishPulls)));
}

float UKoiSkillTreeComponent::GetTurnInMultiplier() const
{
	return FMath::Max(1.0f, 1.0f + SumEffectValue(ENyxKoiSkillEffect::TurnInMultiplier));
}

void UKoiSkillTreeComponent::ApplySkillModifiers(UFishingComponent* FishingComponent, AStarwell* Starwell) const
{
	if (FishingComponent != nullptr)
	{
		FishingComponent->SetFishingSkillModifiers(GetPlayerPowerMultiplier(), GetBiteTimeMultiplier(), GetBonusFishPulls());
	}

	if (Starwell != nullptr)
	{
		Starwell->SetTurnInMultiplier(GetTurnInMultiplier());
	}
}

void UKoiSkillTreeComponent::RestoreSavedState(const TMap<FName, int32>& RestoredSkillRanks)
{
	InvestedSkillRanks.Reset();
	for (const TPair<FName, int32>& SkillRank : RestoredSkillRanks)
	{
		if (!SkillRank.Key.IsNone() && SkillRank.Value > 0)
		{
			InvestedSkillRanks.Add(SkillRank.Key, SkillRank.Value);
		}
	}

	OnKoiSkillTreeRestored.Broadcast(this);
}

FName UKoiSkillTreeComponent::GetSkillSaveKey(const UKoiSkillDataAsset* Skill) const
{
	return Skill != nullptr ? Skill->GetStableSkillId() : NAME_None;
}

bool UKoiSkillTreeComponent::PrerequisitesMet(const UKoiSkillDataAsset* Skill) const
{
	if (Skill == nullptr)
	{
		return false;
	}

	for (const FName& PrerequisiteSkillId : Skill->PrerequisiteSkillIds)
	{
		if (!PrerequisiteSkillId.IsNone() && GetSkillRankById(PrerequisiteSkillId) <= 0)
		{
			return false;
		}
	}

	return true;
}

float UKoiSkillTreeComponent::SumEffectValue(ENyxKoiSkillEffect Effect) const
{
	float TotalValue = 0.0f;
	for (UKoiSkillDataAsset* Skill : AvailableSkills)
	{
		if (Skill != nullptr && Skill->Effect == Effect)
		{
			TotalValue += Skill->EffectValuePerRank * GetSkillRank(Skill);
		}
	}

	return TotalValue;
}
