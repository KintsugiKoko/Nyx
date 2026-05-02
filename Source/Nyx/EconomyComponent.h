#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "UpgradeDataAsset.h"
#include "EconomyComponent.generated.h"

class UEconomyComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FResourceChangedSignature, UEconomyComponent*, EconomyComponent, ENyxResourceType, ResourceType, int32, NewAmount, int32, Delta);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FUpgradeAppliedSignature, UEconomyComponent*, EconomyComponent, UUpgradeDataAsset*, Upgrade);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FEconomyComponentEventSignature, UEconomyComponent*, EconomyComponent);

UCLASS(ClassGroup=(Economy), Blueprintable, meta=(BlueprintSpawnableComponent))
class NYX_API UEconomyComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UEconomyComponent();

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Economy|Resources", meta=(ClampMin="0", UIMin="0"))
	int32 Stardust;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Economy|Resources", meta=(ClampMin="0", UIMin="0"))
	int32 MoonPearls;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Economy|Resources", meta=(ClampMin="0", UIMin="0"))
	int32 EchoScales;

	// Save-friendly applied upgrade state keyed by each upgrade asset's stable UpgradeId.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Economy|Upgrades")
	TMap<FName, int32> AppliedUpgradeCounts;

	UPROPERTY(BlueprintAssignable, Category="Economy|Events")
	FResourceChangedSignature OnResourceChanged;

	UPROPERTY(BlueprintAssignable, Category="Economy|Events")
	FUpgradeAppliedSignature OnUpgradeApplied;

	UPROPERTY(BlueprintAssignable, Category="Economy|Events")
	FEconomyComponentEventSignature OnEconomySaveApplied;

	UFUNCTION(BlueprintCallable, Category="Economy|Resources")
	int32 AddResource(ENyxResourceType ResourceType, int32 Amount);

	UFUNCTION(BlueprintCallable, Category="Economy|Resources")
	bool SpendResource(ENyxResourceType ResourceType, int32 Amount);

	UFUNCTION(BlueprintPure, Category="Economy|Resources")
	int32 GetResourceAmount(ENyxResourceType ResourceType) const;

	UFUNCTION(BlueprintPure, Category="Economy|Upgrades")
	bool CanAfford(const UUpgradeDataAsset* Upgrade) const;

	UFUNCTION(BlueprintPure, Category="Economy|Resources")
	bool CanAffordCosts(const TArray<FNyxResourceAmount>& Costs) const;

	UFUNCTION(BlueprintCallable, Category="Economy|Upgrades")
	bool ApplyUpgrade(UUpgradeDataAsset* Upgrade);

	UFUNCTION(BlueprintCallable, Category="Economy|Save")
	void RestoreSavedState(int32 RestoredStardust, int32 RestoredMoonPearls, int32 RestoredEchoScales, const TMap<FName, int32>& RestoredAppliedUpgradeCounts);

	UFUNCTION(BlueprintPure, Category="Economy|Upgrades")
	int32 GetUpgradeApplyCount(const UUpgradeDataAsset* Upgrade) const;

	UFUNCTION(BlueprintPure, Category="Economy|Upgrades")
	bool HasAppliedUpgrade(const UUpgradeDataAsset* Upgrade) const;

private:
	void SetResourceAmount(ENyxResourceType ResourceType, int32 NewAmount);
	bool SpendCosts(const TArray<FNyxResourceAmount>& Costs);
	FName GetUpgradeSaveKey(const UUpgradeDataAsset* Upgrade) const;
	static bool GetTotalCosts(const TArray<FNyxResourceAmount>& Costs, int64& OutStardustCost, int64& OutMoonPearlsCost, int64& OutEchoScalesCost);
};
