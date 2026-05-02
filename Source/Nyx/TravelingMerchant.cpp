#include "TravelingMerchant.h"

#include "EconomyComponent.h"
#include "UpgradeDataAsset.h"

ATravelingMerchant::ATravelingMerchant()
{
	PrimaryActorTick.bCanEverTick = false;

	MerchantName = FText::FromString(TEXT("Tortoiseshell Traveling Merchant"));
	MerchantDescription = FText::FromString(TEXT("A warm, wandering cat merchant who brings gradual upgrades, tools, and tiny reasons to fish one more time."));
	VisualDirectionNote = FText::FromString(TEXT("Simplified tortoiseshell cat silhouette with patchy black, orange, cream, and brown markings. Keep cozy and toy-like."));
}

void ATravelingMerchant::SetUpgradeInventory(const TArray<UUpgradeDataAsset*>& NewInventory)
{
	UpgradeInventory.Reset();
	UpgradeInventory.Reserve(NewInventory.Num());
	for (UUpgradeDataAsset* Upgrade : NewInventory)
	{
		if (Upgrade != nullptr)
		{
			UpgradeInventory.Add(Upgrade);
		}
	}

	OnMerchantInventoryChanged.Broadcast(this);
}

bool ATravelingMerchant::CanPurchaseUpgrade(UUpgradeDataAsset* Upgrade, const UEconomyComponent* EconomyComponent) const
{
	return Upgrade != nullptr && EconomyComponent != nullptr && UpgradeInventory.Contains(Upgrade) && EconomyComponent->CanAfford(Upgrade);
}

bool ATravelingMerchant::PurchaseUpgrade(UUpgradeDataAsset* Upgrade, UEconomyComponent* EconomyComponent)
{
	if (!CanPurchaseUpgrade(Upgrade, EconomyComponent))
	{
		return false;
	}

	if (!EconomyComponent->ApplyUpgrade(Upgrade))
	{
		return false;
	}

	OnUpgradePurchased.Broadcast(this, Upgrade, EconomyComponent, EconomyComponent->GetUpgradeApplyCount(Upgrade));
	return true;
}
