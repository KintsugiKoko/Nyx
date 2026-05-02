#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameplayTagContainer.h"
#include "NyxGameplayValidation.h"
#include "NyxGameplayDebugActor.generated.h"

class ANyxGameplayDebugActor;
class AStarwell;
class UCardDataAsset;
class UDeckComponent;
class UEconomyComponent;
class UFishDataAsset;
class UFishingComponent;
class UKoiSkillTreeComponent;
class UNyxSaveGame;
class UNyxFTUEComponent;
class USceneComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNyxDebugActionSignature, ANyxGameplayDebugActor*, DebugActor, bool, bSucceeded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FNyxDebugSaveSignature, ANyxGameplayDebugActor*, DebugActor, const FString&, SlotName, bool, bSucceeded);

UCLASS(Blueprintable)
class NYX_API ANyxGameplayDebugActor : public AActor
{
	GENERATED_BODY()

public:
	ANyxGameplayDebugActor();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nyx|Debug")
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nyx|Debug")
	TObjectPtr<UFishingComponent> FishingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nyx|Debug")
	TObjectPtr<UEconomyComponent> EconomyComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nyx|Debug")
	TObjectPtr<UDeckComponent> DeckComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nyx|Debug")
	TObjectPtr<UKoiSkillTreeComponent> SkillTreeComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nyx|Debug")
	TObjectPtr<UNyxFTUEComponent> FTUEComponent;

	// Optional level-placed Starwell. If unset, this actor can spawn one at runtime for PIE tests.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nyx|Debug|Starwell")
	TObjectPtr<AStarwell> StarwellTarget;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nyx|Debug|Starwell")
	bool bSpawnStarwellOnBeginPlay;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nyx|Debug|Save")
	FString SaveSlotName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nyx|Debug|Save")
	int32 SaveUserIndex;

	UPROPERTY(BlueprintAssignable, Category="Nyx|Debug|Events")
	FNyxDebugActionSignature OnDebugActionCompleted;

	UPROPERTY(BlueprintAssignable, Category="Nyx|Debug|Events")
	FNyxDebugSaveSignature OnDebugSaveCompleted;

	UPROPERTY(BlueprintAssignable, Category="Nyx|Debug|Events")
	FNyxDebugSaveSignature OnDebugLoadCompleted;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Nyx|Debug|Validation")
	int32 ObservedFishingRestoreEvents;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Nyx|Debug|Validation")
	int32 ObservedEconomyRestoreEvents;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Nyx|Debug|Validation")
	int32 ObservedDeckRestoreEvents;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Nyx|Debug|Validation")
	int32 ObservedStarwellRestoreEvents;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Nyx|Debug|Validation")
	int32 ObservedThresholdReachedEvents;

	// Blueprint-readable trace of the last save/load reliability validation run for PIE debugging.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Nyx|Debug|Validation")
	TArray<FString> LastSaveLoadReliabilityValidationLog;

	UFUNCTION(BlueprintCallable, Category="Nyx|Debug|Setup")
	void RefreshRewardTargets();

	UFUNCTION(BlueprintPure, Category="Nyx|Debug|Setup")
	AStarwell* GetResolvedStarwell() const;

	UFUNCTION(BlueprintCallable, Category="Nyx|Debug|Economy")
	void DebugAddResources(int32 StardustAmount = 10, int32 MoonPearlsAmount = 1, int32 EchoScalesAmount = 0);

	UFUNCTION(BlueprintCallable, Category="Nyx|Debug|Deck")
	UCardDataAsset* DebugDrawCard();

	UFUNCTION(BlueprintCallable, Category="Nyx|Debug|Deck")
	bool DebugPlayFirstCardInHand();

	UFUNCTION(BlueprintCallable, Category="Nyx|Debug|Deck")
	void DebugShuffleDiscardIntoDeck();

	UFUNCTION(BlueprintCallable, Category="Nyx|Debug|Fishing")
	bool DebugStartCast();

	UFUNCTION(BlueprintCallable, Category="Nyx|Debug|Fishing")
	bool DebugForceFishBite();

	UFUNCTION(BlueprintCallable, Category="Nyx|Debug|Fishing")
	bool DebugStartReel();

	UFUNCTION(BlueprintCallable, Category="Nyx|Debug|Fishing")
	void DebugCompleteCatch(bool bPerfectCatch = false);

	UFUNCTION(BlueprintCallable, Category="Nyx|Debug|Fishing")
	void DebugFailCatch();

	UFUNCTION(BlueprintCallable, Category="Nyx|Debug|Starwell")
	int32 DebugOfferFishToStarwell(UFishDataAsset* Fish);

	UFUNCTION(BlueprintCallable, Category="Nyx|Debug|Save")
	UNyxSaveGame* DebugCaptureSaveGame();

	UFUNCTION(BlueprintCallable, Category="Nyx|Debug|Save")
	bool DebugApplySaveGame(UNyxSaveGame* SaveGame);

	UFUNCTION(BlueprintCallable, Category="Nyx|Debug|Save")
	bool DebugSaveToSlot();

	UFUNCTION(BlueprintCallable, Category="Nyx|Debug|Save")
	bool DebugLoadFromSlot();

	UFUNCTION(BlueprintCallable, Category="Nyx|Debug|Validation")
	FNyxGameplayValidationResult DebugRunGameplayValidations();

	UFUNCTION(BlueprintCallable, Category="Nyx|Debug|Validation")
	FNyxGameplayValidationResult DebugValidateSaveLoadReliability();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<AStarwell> SpawnedStarwell;

	AStarwell* EnsureStarwell();
	void BroadcastActionResult(bool bSucceeded);
	void BindRestoreEventObservers();
	void ResetObservedEventCounts();
	void RecordSaveLoadReliabilityLog(const FString& Message);
	void HandleFishingStateRestored(UFishingComponent* RestoredFishingComponent);
	void HandleEconomyStateRestored(UEconomyComponent* RestoredEconomyComponent);
	void HandleDeckStateRestored(UDeckComponent* RestoredDeckComponent);
	void HandleStarwellStateRestored(AStarwell* RestoredStarwell);
	void HandleStarwellThresholdReached(AStarwell* RestoredStarwell, FName StoryUnlockId, int32 RequiredProgress, const FGameplayTagContainer& StoryUnlockTags);
};
