#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TravelingMerchant.generated.h"

class ATravelingMerchant;
class UEconomyComponent;
class UUpgradeDataAsset;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FTravelingMerchantEventSignature, ATravelingMerchant*, Merchant);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FTravelingMerchantUpgradePurchasedSignature, ATravelingMerchant*, Merchant, UUpgradeDataAsset*, Upgrade, UEconomyComponent*, EconomyComponent, int32, NewApplyCount);

UCLASS(Blueprintable)
class NYX_API ATravelingMerchant : public AActor
{
	GENERATED_BODY()

public:
	ATravelingMerchant();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Merchant")
	FText MerchantName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Merchant", meta=(MultiLine="true"))
	FText MerchantDescription;

	// Source-only identity note for the first pass: this merchant is a tortoiseshell cat, not final art.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Merchant|Presentation")
	FText VisualDirectionNote;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Merchant|Inventory")
	TArray<TObjectPtr<UUpgradeDataAsset>> UpgradeInventory;

	UPROPERTY(BlueprintAssignable, Category="Merchant|Events")
	FTravelingMerchantEventSignature OnMerchantInventoryChanged;

	UPROPERTY(BlueprintAssignable, Category="Merchant|Events")
	FTravelingMerchantUpgradePurchasedSignature OnUpgradePurchased;

	UFUNCTION(BlueprintCallable, Category="Merchant|Inventory")
	void SetUpgradeInventory(const TArray<UUpgradeDataAsset*>& NewInventory);

	UFUNCTION(BlueprintPure, Category="Merchant|Inventory")
	bool CanPurchaseUpgrade(UUpgradeDataAsset* Upgrade, const UEconomyComponent* EconomyComponent) const;

	UFUNCTION(BlueprintCallable, Category="Merchant|Inventory")
	bool PurchaseUpgrade(UUpgradeDataAsset* Upgrade, UEconomyComponent* EconomyComponent);
};
