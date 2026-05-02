#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "UpgradeDataAsset.generated.h"

UENUM(BlueprintType)
enum class ENyxResourceType : uint8
{
	Stardust UMETA(DisplayName="Stardust"),
	MoonPearls UMETA(DisplayName="Moon Pearls"),
	EchoScales UMETA(DisplayName="Echo Scales"),
	Koi UMETA(DisplayName="Koi")
};

USTRUCT(BlueprintType)
struct FNyxResourceAmount
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resources")
	ENyxResourceType ResourceType = ENyxResourceType::Stardust;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Resources", meta=(ClampMin="0", UIMin="0"))
	int32 Amount = 0;
};

UENUM(BlueprintType)
enum class ENyxUpgradePrototypeEffect : uint8
{
	None UMETA(DisplayName="None"),
	IncreaseStardustValue UMETA(DisplayName="Increase Stardust Value"),
	ReduceBiteTime UMETA(DisplayName="Reduce Bite Time"),
	ReduceReelDifficulty UMETA(DisplayName="Reduce Reel Difficulty"),
	IncreasePlayerPower UMETA(DisplayName="Increase Player Power"),
	AddBonusFishPull UMETA(DisplayName="Add Bonus Fish Pull"),
	IncreaseTurnInMultiplier UMETA(DisplayName="Increase Turn-In Multiplier"),
	UnlockGatheringTool UMETA(DisplayName="Unlock Gathering Tool"),
	IncreaseHandSize UMETA(DisplayName="Increase Hand Size"),
	AddStartingCard UMETA(DisplayName="Add Starting Card"),
	UnlockFishingTag UMETA(DisplayName="Unlock Fishing Tag")
};

UCLASS(BlueprintType)
class NYX_API UUpgradeDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Stable save/load key. Keep this unchanged after an upgrade ships.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Upgrade")
	FName UpgradeId;

	// Player-facing upgrade name shown in menus and reward screens.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Upgrade")
	FText DisplayName;

	// Rules text or flavor text shown when inspecting the upgrade.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Upgrade", meta=(MultiLine="true"))
	FText Description;

	// Resource costs required before this upgrade can be applied.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Upgrade|Cost")
	TArray<FNyxResourceAmount> Costs;

	// Number of times this upgrade can be applied. Use 1 for a normal permanent unlock.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Upgrade", meta=(ClampMin="1", UIMin="1"))
	int32 MaxApplications = 1;

	// Prototype effect hook for early balancing before upgrades become full effect objects.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Upgrade|Effects")
	ENyxUpgradePrototypeEffect PrototypeEffect = ENyxUpgradePrototypeEffect::None;

	// Tags affected or unlocked by this upgrade, such as Fish.Night or Card.Lure.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Upgrade|Effects")
	FGameplayTagContainer AffectedGameplayTags;
};
