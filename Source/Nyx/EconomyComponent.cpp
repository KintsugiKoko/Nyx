#include "EconomyComponent.h"

UEconomyComponent::UEconomyComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	Stardust = 0;
	MoonPearls = 0;
	EchoScales = 0;
}

int32 UEconomyComponent::AddResource(ENyxResourceType ResourceType, int32 Amount)
{
	if (Amount <= 0)
	{
		return GetResourceAmount(ResourceType);
	}

	const int32 OldAmount = GetResourceAmount(ResourceType);
	const int32 NewAmount = FMath::Clamp<int64>(static_cast<int64>(OldAmount) + Amount, 0, MAX_int32);
	SetResourceAmount(ResourceType, NewAmount);
	return NewAmount;
}

bool UEconomyComponent::SpendResource(ENyxResourceType ResourceType, int32 Amount)
{
	if (Amount < 0)
	{
		return false;
	}

	if (Amount == 0)
	{
		return true;
	}

	const int32 OldAmount = GetResourceAmount(ResourceType);
	if (OldAmount < Amount)
	{
		return false;
	}

	SetResourceAmount(ResourceType, OldAmount - Amount);
	return true;
}

int32 UEconomyComponent::GetResourceAmount(ENyxResourceType ResourceType) const
{
	switch (ResourceType)
	{
	case ENyxResourceType::Stardust:
		return Stardust;
	case ENyxResourceType::MoonPearls:
		return MoonPearls;
	case ENyxResourceType::EchoScales:
		return EchoScales;
	default:
		return 0;
	}
}

bool UEconomyComponent::CanAfford(const UUpgradeDataAsset* Upgrade) const
{
	if (Upgrade == nullptr)
	{
		return false;
	}

	const int32 CurrentApplyCount = GetUpgradeApplyCount(Upgrade);
	if (CurrentApplyCount >= Upgrade->MaxApplications)
	{
		return false;
	}

	return CanAffordCosts(Upgrade->Costs);
}

bool UEconomyComponent::CanAffordCosts(const TArray<FNyxResourceAmount>& Costs) const
{
	int64 StardustCost = 0;
	int64 MoonPearlsCost = 0;
	int64 EchoScalesCost = 0;

	if (!GetTotalCosts(Costs, StardustCost, MoonPearlsCost, EchoScalesCost))
	{
		return false;
	}

	return Stardust >= StardustCost && MoonPearls >= MoonPearlsCost && EchoScales >= EchoScalesCost;
}

bool UEconomyComponent::ApplyUpgrade(UUpgradeDataAsset* Upgrade)
{
	if (!CanAfford(Upgrade))
	{
		return false;
	}

	if (!SpendCosts(Upgrade->Costs))
	{
		return false;
	}

	const FName UpgradeSaveKey = GetUpgradeSaveKey(Upgrade);
	if (UpgradeSaveKey.IsNone())
	{
		return false;
	}

	int32& ApplyCount = AppliedUpgradeCounts.FindOrAdd(UpgradeSaveKey);
	++ApplyCount;

	OnUpgradeApplied.Broadcast(this, Upgrade);
	return true;
}

void UEconomyComponent::RestoreSavedState(int32 RestoredStardust, int32 RestoredMoonPearls, int32 RestoredEchoScales, const TMap<FName, int32>& RestoredAppliedUpgradeCounts)
{
	SetResourceAmount(ENyxResourceType::Stardust, RestoredStardust);
	SetResourceAmount(ENyxResourceType::MoonPearls, RestoredMoonPearls);
	SetResourceAmount(ENyxResourceType::EchoScales, RestoredEchoScales);

	AppliedUpgradeCounts.Reset();
	for (const TPair<FName, int32>& UpgradeCount : RestoredAppliedUpgradeCounts)
	{
		if (!UpgradeCount.Key.IsNone() && UpgradeCount.Value > 0)
		{
			AppliedUpgradeCounts.Add(UpgradeCount.Key, UpgradeCount.Value);
		}
	}

	OnEconomySaveApplied.Broadcast(this);
}

int32 UEconomyComponent::GetUpgradeApplyCount(const UUpgradeDataAsset* Upgrade) const
{
	const FName UpgradeSaveKey = GetUpgradeSaveKey(Upgrade);
	if (UpgradeSaveKey.IsNone())
	{
		return 0;
	}

	if (const int32* ApplyCount = AppliedUpgradeCounts.Find(UpgradeSaveKey))
	{
		return *ApplyCount;
	}

	return 0;
}

bool UEconomyComponent::HasAppliedUpgrade(const UUpgradeDataAsset* Upgrade) const
{
	return GetUpgradeApplyCount(Upgrade) > 0;
}

void UEconomyComponent::SetResourceAmount(ENyxResourceType ResourceType, int32 NewAmount)
{
	NewAmount = FMath::Max(0, NewAmount);

	int32* ResourceValue = nullptr;
	switch (ResourceType)
	{
	case ENyxResourceType::Stardust:
		ResourceValue = &Stardust;
		break;
	case ENyxResourceType::MoonPearls:
		ResourceValue = &MoonPearls;
		break;
	case ENyxResourceType::EchoScales:
		ResourceValue = &EchoScales;
		break;
	default:
		return;
	}

	const int32 OldAmount = *ResourceValue;
	if (OldAmount == NewAmount)
	{
		return;
	}

	*ResourceValue = NewAmount;
	OnResourceChanged.Broadcast(this, ResourceType, NewAmount, NewAmount - OldAmount);
}

bool UEconomyComponent::SpendCosts(const TArray<FNyxResourceAmount>& Costs)
{
	int64 StardustCost = 0;
	int64 MoonPearlsCost = 0;
	int64 EchoScalesCost = 0;

	if (!GetTotalCosts(Costs, StardustCost, MoonPearlsCost, EchoScalesCost))
	{
		return false;
	}

	if (Stardust < StardustCost || MoonPearls < MoonPearlsCost || EchoScales < EchoScalesCost)
	{
		return false;
	}

	SetResourceAmount(ENyxResourceType::Stardust, Stardust - static_cast<int32>(StardustCost));
	SetResourceAmount(ENyxResourceType::MoonPearls, MoonPearls - static_cast<int32>(MoonPearlsCost));
	SetResourceAmount(ENyxResourceType::EchoScales, EchoScales - static_cast<int32>(EchoScalesCost));
	return true;
}

FName UEconomyComponent::GetUpgradeSaveKey(const UUpgradeDataAsset* Upgrade) const
{
	if (Upgrade == nullptr)
	{
		return NAME_None;
	}

	return Upgrade->UpgradeId.IsNone() ? Upgrade->GetFName() : Upgrade->UpgradeId;
}

bool UEconomyComponent::GetTotalCosts(const TArray<FNyxResourceAmount>& Costs, int64& OutStardustCost, int64& OutMoonPearlsCost, int64& OutEchoScalesCost)
{
	OutStardustCost = 0;
	OutMoonPearlsCost = 0;
	OutEchoScalesCost = 0;

	for (const FNyxResourceAmount& Cost : Costs)
	{
		if (Cost.Amount < 0)
		{
			return false;
		}

		switch (Cost.ResourceType)
		{
		case ENyxResourceType::Stardust:
			OutStardustCost += Cost.Amount;
			break;
		case ENyxResourceType::MoonPearls:
			OutMoonPearlsCost += Cost.Amount;
			break;
		case ENyxResourceType::EchoScales:
			OutEchoScalesCost += Cost.Amount;
			break;
		default:
			return false;
		}
	}

	return OutStardustCost <= MAX_int32 && OutMoonPearlsCost <= MAX_int32 && OutEchoScalesCost <= MAX_int32;
}
