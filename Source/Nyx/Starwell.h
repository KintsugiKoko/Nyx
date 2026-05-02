#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "Starwell.generated.h"

class AStarwell;
class UEconomyComponent;
class UFishDataAsset;
class USceneComponent;
class USphereComponent;
class UStaticMeshComponent;

USTRUCT(BlueprintType)
struct FStarwellOfferingThreshold
{
	GENERATED_BODY()

	// Offering progress required before this threshold fires.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Starwell", meta=(ClampMin="0", UIMin="0"))
	int32 RequiredProgress = 0;

	// Stable key for save/load and story systems. Keep this unchanged after content ships.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Starwell")
	FName StoryUnlockId;

	// Tags for future story routing, quest unlocks, scenes, or world-state changes.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Starwell")
	FGameplayTagContainer StoryUnlockTags;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FStarwellFishAcceptedSignature, AStarwell*, Starwell, UFishDataAsset*, Fish, int32, EchoScalesGranted, int32, NewOfferingProgress, UEconomyComponent*, EconomyComponent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FStarwellProgressChangedSignature, AStarwell*, Starwell, int32, NewOfferingProgress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FStarwellThresholdReachedSignature, AStarwell*, Starwell, FName, StoryUnlockId, int32, RequiredProgress, const FGameplayTagContainer&, StoryUnlockTags);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FStarwellStoryUnlockSignature, AStarwell*, Starwell, FName, StoryUnlockId, const FGameplayTagContainer&, StoryUnlockTags);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FStarwellEventSignature, AStarwell*, Starwell);
DECLARE_MULTICAST_DELEGATE_OneParam(FStarwellNativeEventSignature, AStarwell*);
DECLARE_MULTICAST_DELEGATE_FourParams(FStarwellNativeThresholdReachedSignature, AStarwell*, FName, int32, const FGameplayTagContainer&);

UCLASS(Blueprintable)
class NYX_API AStarwell : public AActor
{
	GENERATED_BODY()

public:
	AStarwell();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Starwell|Components")
	TObjectPtr<USceneComponent> SceneRoot;

	// Assign a chunky basin mesh in Blueprint or a child class. Kept asset-free in C++.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Starwell|Components")
	TObjectPtr<UStaticMeshComponent> VisualMesh;

	// Overlap/query area for player interaction prompts or offering input.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Starwell|Components")
	TObjectPtr<USphereComponent> InteractionCollision;

	// Where offered fish props can move toward before conversion VFX.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Starwell|Components")
	TObjectPtr<USceneComponent> OfferingPoint;

	// Central spawn point for idle pulse, offering accepted, threshold reached, and post-load refresh VFX.
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Starwell|Components")
	TObjectPtr<USceneComponent> VfxSpawnPoint;

	// Progress thresholds that can unlock future story beats when fish offerings reach them.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Starwell|Story")
	TArray<FStarwellOfferingThreshold> OfferingThresholds;

	// Each point of a fish's BaseStardustValue becomes this many Echo Scales.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Starwell|Conversion", meta=(ClampMin="0", UIMin="0"))
	int32 EchoScalesPerBaseStardustValue;

	// Ensures even low-value prototype fish can still produce a useful offering reward.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Starwell|Conversion", meta=(ClampMin="0", UIMin="0"))
	int32 MinimumEchoScalesPerFish;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Starwell|Conversion", meta=(ClampMin="0.1", UIMin="0.1"))
	float TurnInMultiplier;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Starwell|Progress", meta=(ClampMin="0", UIMin="0"))
	int32 OfferingProgress;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Starwell|Progress", meta=(ClampMin="0", UIMin="0"))
	int32 TotalFishAccepted;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Starwell|Progress", meta=(ClampMin="0", UIMin="0"))
	int32 TotalEchoScalesGenerated;

	// Save-friendly list of story threshold IDs that have already fired.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Starwell|Story")
	TArray<FName> ReachedStoryUnlockIds;

	UPROPERTY(BlueprintAssignable, Category="Starwell|Events")
	FStarwellFishAcceptedSignature OnFishAccepted;

	UPROPERTY(BlueprintAssignable, Category="Starwell|Events")
	FStarwellProgressChangedSignature OnOfferingProgressChanged;

	UPROPERTY(BlueprintAssignable, Category="Starwell|Events")
	FStarwellThresholdReachedSignature OnOfferingThresholdReached;

	UPROPERTY(BlueprintAssignable, Category="Starwell|Events")
	FStarwellStoryUnlockSignature OnStoryUnlockAvailable;

	UPROPERTY(BlueprintAssignable, Category="Starwell|Events")
	FStarwellEventSignature OnStarwellSaveApplied;

	UPROPERTY(BlueprintAssignable, Category="Starwell|Events")
	FStarwellEventSignature OnStarwellStateRestored;

	// Native mirrors for C++ tests and validation; Blueprint presentation should bind to the BlueprintAssignable events above.
	FStarwellNativeEventSignature OnStarwellStateRestoredNative;
	FStarwellNativeThresholdReachedSignature OnOfferingThresholdReachedNative;

	UFUNCTION(BlueprintCallable, Category="Starwell")
	int32 AcceptCaughtFish(UFishDataAsset* Fish, UEconomyComponent* EconomyComponent);

	UFUNCTION(BlueprintPure, Category="Starwell")
	int32 CalculateEchoScalesForFish(UFishDataAsset* Fish) const;

	UFUNCTION(BlueprintPure, Category="Starwell")
	bool HasReachedStoryUnlock(FName StoryUnlockId) const;

	UFUNCTION(BlueprintPure, Category="Starwell")
	float GetProgressToNextThreshold() const;

	UFUNCTION(BlueprintCallable, Category="Starwell|Save")
	void RestoreSavedProgress(int32 RestoredEchoScalesPerBaseStardustValue, int32 RestoredMinimumEchoScalesPerFish, float RestoredTurnInMultiplier, int32 RestoredOfferingProgress, int32 RestoredTotalFishAccepted, int32 RestoredTotalEchoScalesGenerated, const TArray<FName>& RestoredReachedStoryUnlockIds);

	UFUNCTION(BlueprintCallable, Category="Starwell|Conversion")
	void SetTurnInMultiplier(float NewTurnInMultiplier);

	UFUNCTION(BlueprintCallable, Category="Starwell|Validation")
	bool ValidateOfferingThresholds(UPARAM(ref) TArray<FString>& OutFailures) const;

private:
	void AddOfferingProgress(int32 ProgressDelta);
	void CheckOfferingThresholds();
	FName GetThresholdSaveKey(const FStarwellOfferingThreshold& Threshold) const;
	TArray<FStarwellOfferingThreshold> GetReachedThresholdsInOrder() const;
};
