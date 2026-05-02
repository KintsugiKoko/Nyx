#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "KoiSkillDataAsset.generated.h"

UENUM(BlueprintType)
enum class ENyxKoiSkillEffect : uint8
{
	PlayerPower UMETA(DisplayName="Player Power"),
	FasterBites UMETA(DisplayName="Faster Bites"),
	BonusFishPulls UMETA(DisplayName="Bonus Fish Pulls"),
	TurnInMultiplier UMETA(DisplayName="Turn-In Multiplier")
};

UCLASS(BlueprintType)
class NYX_API UKoiSkillDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Stable save/load key. Keep this unchanged once a skill ships.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Koi Skill")
	FName SkillId;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Koi Skill")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Koi Skill", meta=(MultiLine="true"))
	FText Description;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Koi Skill", meta=(ClampMin="1", UIMin="1"))
	int32 MaxRank = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Koi Skill|Cost", meta=(ClampMin="0", UIMin="0"))
	int32 BaseKoiCost = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Koi Skill|Cost", meta=(ClampMin="0", UIMin="0"))
	int32 KoiCostIncreasePerRank = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Koi Skill|Effect")
	ENyxKoiSkillEffect Effect = ENyxKoiSkillEffect::PlayerPower;

	// Meaning depends on Effect. For FasterBites this is a percentage point reduction per rank.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Koi Skill|Effect")
	float EffectValuePerRank = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Koi Skill|Rules")
	TArray<FName> PrerequisiteSkillIds;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Koi Skill|Tags")
	FGameplayTagContainer SkillTags;

	UFUNCTION(BlueprintPure, Category="Koi Skill")
	FName GetStableSkillId() const;

	UFUNCTION(BlueprintPure, Category="Koi Skill")
	int32 GetKoiCostForRank(int32 CurrentRank) const;
};
