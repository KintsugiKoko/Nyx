#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "KoiSkillDataAsset.h"
#include "KoiSkillTreeComponent.generated.h"

class AStarwell;
class UEconomyComponent;
class UFishingComponent;
class UKoiSkillTreeComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FKoiSkillRankChangedSignature, UKoiSkillTreeComponent*, SkillTreeComponent, UKoiSkillDataAsset*, Skill, int32, NewRank, int32, KoiSpent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKoiSkillTreeEventSignature, UKoiSkillTreeComponent*, SkillTreeComponent);

UCLASS(ClassGroup=(Nyx), Blueprintable, meta=(BlueprintSpawnableComponent))
class NYX_API UKoiSkillTreeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UKoiSkillTreeComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Koi Skills")
	TArray<TObjectPtr<UKoiSkillDataAsset>> AvailableSkills;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Koi Skills")
	TMap<FName, int32> InvestedSkillRanks;

	UPROPERTY(BlueprintAssignable, Category="Koi Skills|Events")
	FKoiSkillRankChangedSignature OnKoiSkillRankChanged;

	UPROPERTY(BlueprintAssignable, Category="Koi Skills|Events")
	FKoiSkillTreeEventSignature OnKoiSkillTreeRestored;

	UFUNCTION(BlueprintCallable, Category="Koi Skills")
	bool InvestSkill(UKoiSkillDataAsset* Skill, UEconomyComponent* EconomyComponent);

	UFUNCTION(BlueprintPure, Category="Koi Skills")
	bool CanInvestSkill(const UKoiSkillDataAsset* Skill, const UEconomyComponent* EconomyComponent) const;

	UFUNCTION(BlueprintPure, Category="Koi Skills")
	int32 GetSkillRank(const UKoiSkillDataAsset* Skill) const;

	UFUNCTION(BlueprintPure, Category="Koi Skills")
	int32 GetSkillRankById(FName SkillId) const;

	UFUNCTION(BlueprintPure, Category="Koi Skills")
	int32 GetNextRankKoiCost(const UKoiSkillDataAsset* Skill) const;

	UFUNCTION(BlueprintPure, Category="Koi Skills")
	float GetPlayerPowerMultiplier() const;

	UFUNCTION(BlueprintPure, Category="Koi Skills")
	float GetBiteTimeMultiplier() const;

	UFUNCTION(BlueprintPure, Category="Koi Skills")
	int32 GetBonusFishPulls() const;

	UFUNCTION(BlueprintPure, Category="Koi Skills")
	float GetTurnInMultiplier() const;

	UFUNCTION(BlueprintCallable, Category="Koi Skills")
	void ApplySkillModifiers(UFishingComponent* FishingComponent, AStarwell* Starwell) const;

	UFUNCTION(BlueprintCallable, Category="Koi Skills|Save")
	void RestoreSavedState(const TMap<FName, int32>& RestoredSkillRanks);

private:
	FName GetSkillSaveKey(const UKoiSkillDataAsset* Skill) const;
	bool PrerequisitesMet(const UKoiSkillDataAsset* Skill) const;
	float SumEffectValue(ENyxKoiSkillEffect Effect) const;
};
