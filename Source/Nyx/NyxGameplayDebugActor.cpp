#include "NyxGameplayDebugActor.h"

#include "CardDataAsset.h"
#include "Components/SceneComponent.h"
#include "DeckComponent.h"
#include "EconomyComponent.h"
#include "Engine/World.h"
#include "FishDataAsset.h"
#include "FishingComponent.h"
#include "NyxGameplayValidation.h"
#include "NyxSaveGame.h"
#include "Starwell.h"

ANyxGameplayDebugActor::ANyxGameplayDebugActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	FishingComponent = CreateDefaultSubobject<UFishingComponent>(TEXT("FishingComponent"));
	EconomyComponent = CreateDefaultSubobject<UEconomyComponent>(TEXT("EconomyComponent"));
	DeckComponent = CreateDefaultSubobject<UDeckComponent>(TEXT("DeckComponent"));

	StarwellTarget = nullptr;
	SpawnedStarwell = nullptr;
	bSpawnStarwellOnBeginPlay = true;
	SaveSlotName = TEXT("NyxDebugSlot");
	SaveUserIndex = 0;
	ResetObservedEventCounts();
}

void ANyxGameplayDebugActor::BeginPlay()
{
	Super::BeginPlay();
	RefreshRewardTargets();
	BindRestoreEventObservers();
}

void ANyxGameplayDebugActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (SpawnedStarwell != nullptr)
	{
		SpawnedStarwell->Destroy();
		SpawnedStarwell = nullptr;
	}

	Super::EndPlay(EndPlayReason);
}

void ANyxGameplayDebugActor::RefreshRewardTargets()
{
	if (FishingComponent != nullptr)
	{
		FishingComponent->SetCatchRewardTargets(EnsureStarwell(), EconomyComponent.Get());
	}

	BindRestoreEventObservers();
}

AStarwell* ANyxGameplayDebugActor::GetResolvedStarwell() const
{
	return StarwellTarget != nullptr ? StarwellTarget.Get() : SpawnedStarwell.Get();
}

void ANyxGameplayDebugActor::DebugAddResources(int32 StardustAmount, int32 MoonPearlsAmount, int32 EchoScalesAmount)
{
	if (EconomyComponent == nullptr)
	{
		BroadcastActionResult(false);
		return;
	}

	EconomyComponent->AddResource(ENyxResourceType::Stardust, StardustAmount);
	EconomyComponent->AddResource(ENyxResourceType::MoonPearls, MoonPearlsAmount);
	EconomyComponent->AddResource(ENyxResourceType::EchoScales, EchoScalesAmount);
	BroadcastActionResult(true);
}

UCardDataAsset* ANyxGameplayDebugActor::DebugDrawCard()
{
	if (DeckComponent == nullptr)
	{
		BroadcastActionResult(false);
		return nullptr;
	}

	UCardDataAsset* DrawnCard = DeckComponent->DrawCard();
	BroadcastActionResult(DrawnCard != nullptr);
	return DrawnCard;
}

bool ANyxGameplayDebugActor::DebugPlayFirstCardInHand()
{
	if (DeckComponent == nullptr || DeckComponent->Hand.IsEmpty())
	{
		BroadcastActionResult(false);
		return false;
	}

	const bool bPlayed = DeckComponent->PlayCard(DeckComponent->Hand[0].Get());
	BroadcastActionResult(bPlayed);
	return bPlayed;
}

void ANyxGameplayDebugActor::DebugShuffleDiscardIntoDeck()
{
	if (DeckComponent == nullptr)
	{
		BroadcastActionResult(false);
		return;
	}

	DeckComponent->ShuffleDiscardIntoDeck();
	BroadcastActionResult(true);
}

bool ANyxGameplayDebugActor::DebugStartCast()
{
	if (FishingComponent == nullptr)
	{
		BroadcastActionResult(false);
		return false;
	}

	RefreshRewardTargets();
	const bool bStarted = FishingComponent->StartCast();
	BroadcastActionResult(bStarted);
	return bStarted;
}

bool ANyxGameplayDebugActor::DebugForceFishBite()
{
	if (FishingComponent == nullptr || FishingComponent->FishingState != EFishingState::Casting || FishingComponent->SelectedFish == nullptr)
	{
		BroadcastActionResult(false);
		return false;
	}

	FishingComponent->FishingState = EFishingState::FishBiting;
	FishingComponent->CurrentBiteTime = 0.0f;
	FishingComponent->OnFishingStateChanged.Broadcast(FishingComponent.Get(), EFishingState::FishBiting);
	FishingComponent->OnFishBite.Broadcast(FishingComponent.Get(), FishingComponent->SelectedFish.Get());
	BroadcastActionResult(true);
	return true;
}

bool ANyxGameplayDebugActor::DebugStartReel()
{
	if (FishingComponent == nullptr)
	{
		BroadcastActionResult(false);
		return false;
	}

	const bool bStarted = FishingComponent->StartReel();
	BroadcastActionResult(bStarted);
	return bStarted;
}

void ANyxGameplayDebugActor::DebugCompleteCatch(bool bPerfectCatch)
{
	if (FishingComponent == nullptr)
	{
		BroadcastActionResult(false);
		return;
	}

	RefreshRewardTargets();
	const bool bCanComplete = FishingComponent->FishingState == EFishingState::Reeling && FishingComponent->SelectedFish != nullptr;
	FishingComponent->CompleteCatch(bPerfectCatch);
	BroadcastActionResult(bCanComplete && FishingComponent->FishingState == EFishingState::CatchComplete);
}

void ANyxGameplayDebugActor::DebugFailCatch()
{
	if (FishingComponent == nullptr)
	{
		BroadcastActionResult(false);
		return;
	}

	const bool bCanFail = FishingComponent->IsFishing() || FishingComponent->SelectedFish != nullptr;
	FishingComponent->FailCatch();
	BroadcastActionResult(bCanFail && FishingComponent->FishingState == EFishingState::CatchFailed);
}

int32 ANyxGameplayDebugActor::DebugOfferFishToStarwell(UFishDataAsset* Fish)
{
	if (FishingComponent == nullptr || Fish == nullptr)
	{
		BroadcastActionResult(false);
		return 0;
	}

	RefreshRewardTargets();
	const int32 EchoScalesGranted = FishingComponent->OfferFishToStarwell(Fish);
	BroadcastActionResult(EchoScalesGranted > 0);
	return EchoScalesGranted;
}

UNyxSaveGame* ANyxGameplayDebugActor::DebugCaptureSaveGame()
{
	RefreshRewardTargets();
	UNyxSaveGame* SaveGame = UNyxSaveGameLibrary::CaptureNyxSaveGame(FishingComponent.Get(), EconomyComponent.Get(), DeckComponent.Get(), GetResolvedStarwell());
	BroadcastActionResult(SaveGame != nullptr);
	return SaveGame;
}

bool ANyxGameplayDebugActor::DebugApplySaveGame(UNyxSaveGame* SaveGame)
{
	RefreshRewardTargets();
	const bool bApplied = UNyxSaveGameLibrary::ApplyNyxSaveGame(SaveGame, FishingComponent.Get(), EconomyComponent.Get(), DeckComponent.Get(), GetResolvedStarwell());
	BroadcastActionResult(bApplied);
	return bApplied;
}

bool ANyxGameplayDebugActor::DebugSaveToSlot()
{
	RefreshRewardTargets();
	const bool bSaved = UNyxSaveGameLibrary::SaveNyxGameToSlot(SaveSlotName, SaveUserIndex, FishingComponent.Get(), EconomyComponent.Get(), DeckComponent.Get(), GetResolvedStarwell());
	OnDebugSaveCompleted.Broadcast(this, SaveSlotName, bSaved);
	BroadcastActionResult(bSaved);
	return bSaved;
}

bool ANyxGameplayDebugActor::DebugLoadFromSlot()
{
	RefreshRewardTargets();
	const bool bLoaded = UNyxSaveGameLibrary::LoadNyxGameFromSlotAndApply(SaveSlotName, SaveUserIndex, FishingComponent.Get(), EconomyComponent.Get(), DeckComponent.Get(), GetResolvedStarwell());
	OnDebugLoadCompleted.Broadcast(this, SaveSlotName, bLoaded);
	BroadcastActionResult(bLoaded);
	return bLoaded;
}

FNyxGameplayValidationResult ANyxGameplayDebugActor::DebugRunGameplayValidations()
{
	FNyxGameplayValidationResult Result = UNyxGameplayValidationLibrary::RunAllGameplayValidations(this);
	BroadcastActionResult(Result.bPassed);
	return Result;
}

FNyxGameplayValidationResult ANyxGameplayDebugActor::DebugValidateSaveLoadReliability()
{
	FNyxGameplayValidationResult Result;
	LastSaveLoadReliabilityValidationLog.Reset();
	RecordSaveLoadReliabilityLog(TEXT("Starting Save/Load Reliability validation."));

	auto AddReliabilityFailure = [this, &Result](const FString& Failure)
	{
		Result.AddFailure(Failure);
		RecordSaveLoadReliabilityLog(FString::Printf(TEXT("FAIL: %s"), *Failure));
	};

	if (FishingComponent == nullptr || EconomyComponent == nullptr || DeckComponent == nullptr)
	{
		AddReliabilityFailure(TEXT("Debug actor is missing one or more gameplay components."));
		BroadcastActionResult(false);
		return Result;
	}

	AStarwell* Starwell = GetWorld() != nullptr ? GetWorld()->SpawnActor<AStarwell>(AStarwell::StaticClass(), GetActorTransform()) : nullptr;
	if (Starwell == nullptr)
	{
		AddReliabilityFailure(TEXT("Debug actor could not spawn an isolated Starwell for validation."));
		BroadcastActionResult(false);
		return Result;
	}

	BindRestoreEventObservers();
	Starwell->OnStarwellStateRestoredNative.RemoveAll(this);
	Starwell->OnStarwellStateRestoredNative.AddUObject(this, &ANyxGameplayDebugActor::HandleStarwellStateRestored);
	Starwell->OnOfferingThresholdReachedNative.RemoveAll(this);
	Starwell->OnOfferingThresholdReachedNative.AddUObject(this, &ANyxGameplayDebugActor::HandleStarwellThresholdReached);

	UFishDataAsset* Fish = NewObject<UFishDataAsset>(this, MakeUniqueObjectName(this, UFishDataAsset::StaticClass(), FName(TEXT("DebugReliabilityGlowMinnow"))));
	Fish->FishId = TEXT("DebugReliabilityGlowMinnow");
	Fish->DisplayName = FText::FromString(TEXT("Debug Reliability Glow Minnow"));
	Fish->BaseStardustValue = 5;
	Fish->BiteTimeRange = FVector2D(0.0f, 0.0f);
	Fish->ReelDifficulty = 1.0f;

	FishingComponent->AvailableFish.Reset();
	FishingComponent->AvailableFish.Add(Fish);
	FishingComponent->FishingProgress = FNyxFishingProgressData();
	FishingComponent->SelectedFish = Fish;
	FishingComponent->FishingState = EFishingState::Casting;
	FishingComponent->CurrentBiteTime = 9.0f;
	FishingComponent->SetTension(0.85f);
	FishingComponent->RecordCatchProgress(Fish, true);

	EconomyComponent->RestoreSavedState(0, 0, 0, TMap<FName, int32>());

	Starwell->OfferingThresholds.Reset();
	FStarwellOfferingThreshold Threshold;
	Threshold.RequiredProgress = 5;
	Threshold.StoryUnlockId = TEXT("DebugReliabilityThreshold");
	Starwell->OfferingThresholds.Add(Threshold);
	Starwell->RestoreSavedProgress(1, 1, 0, 0, 0, TArray<FName>());

	ResetObservedEventCounts();

	const int32 EchoScalesGranted = Starwell->AcceptCaughtFish(Fish, EconomyComponent.Get());
	if (EchoScalesGranted != 5 || !Starwell->HasReachedStoryUnlock(TEXT("DebugReliabilityThreshold")))
	{
		AddReliabilityFailure(TEXT("Debug reliability setup did not reach the expected Starwell threshold."));
	}
	RecordSaveLoadReliabilityLog(FString::Printf(TEXT("Setup offering granted %d Echo Scales and reached threshold: %s."),
		EchoScalesGranted,
		Starwell->HasReachedStoryUnlock(TEXT("DebugReliabilityThreshold")) ? TEXT("true") : TEXT("false")));

	UNyxSaveGame* SaveGame = UNyxSaveGameLibrary::CaptureNyxSaveGame(FishingComponent.Get(), EconomyComponent.Get(), DeckComponent.Get(), Starwell);
	if (SaveGame == nullptr)
	{
		AddReliabilityFailure(TEXT("Debug reliability capture failed."));
		Starwell->Destroy();
		BroadcastActionResult(false);
		return Result;
	}
	RecordSaveLoadReliabilityLog(FString::Printf(TEXT("Captured save with Starwell progress=%d, fish accepted=%d, threshold IDs=%d."),
		SaveGame->Starwell.OfferingProgress,
		SaveGame->Starwell.TotalFishAccepted,
		SaveGame->Starwell.ReachedStoryUnlockIds.Num()));

	FishingComponent->SelectedFish = Fish;
	FishingComponent->FishingState = EFishingState::Reeling;
	FishingComponent->CurrentBiteTime = 4.0f;
	FishingComponent->SetTension(1.0f);
	FishingComponent->FishingProgress = FNyxFishingProgressData();
	EconomyComponent->RestoreSavedState(0, 0, 0, TMap<FName, int32>());
	Starwell->RestoreSavedProgress(1, 1, 0, 0, 0, TArray<FName>());

	ResetObservedEventCounts();
	if (!UNyxSaveGameLibrary::ApplyNyxSaveGame(SaveGame, FishingComponent.Get(), EconomyComponent.Get(), DeckComponent.Get(), Starwell))
	{
		AddReliabilityFailure(TEXT("Debug reliability apply failed."));
	}
	RecordSaveLoadReliabilityLog(FString::Printf(TEXT("Observed post-load events: Fishing=%d Economy=%d Deck=%d Starwell=%d Threshold=%d."),
		ObservedFishingRestoreEvents,
		ObservedEconomyRestoreEvents,
		ObservedDeckRestoreEvents,
		ObservedStarwellRestoreEvents,
		ObservedThresholdReachedEvents));

	if (FishingComponent->FishingState != EFishingState::Idle || FishingComponent->SelectedFish != nullptr || FishingComponent->CurrentBiteTime != 0.0f || !FMath::IsNearlyZero(FishingComponent->Tension))
	{
		AddReliabilityFailure(TEXT("Active fishing runtime state did not restore to a clean Idle state."));
	}
	else
	{
		RecordSaveLoadReliabilityLog(TEXT("Active fishing runtime state restored to clean Idle."));
	}

	if (!FishingComponent->HasDiscoveredFish(TEXT("DebugReliabilityGlowMinnow")) || FishingComponent->GetCatchCountForFish(TEXT("DebugReliabilityGlowMinnow")) != 1 || FishingComponent->GetPerfectCatchCountForFish(TEXT("DebugReliabilityGlowMinnow")) != 1)
	{
		AddReliabilityFailure(TEXT("Durable fishing collection progress did not restore correctly."));
	}
	else
	{
		RecordSaveLoadReliabilityLog(TEXT("Durable fishing collection progress restored correctly."));
	}

	int32 ClaimedReliabilityThresholdCount = 0;
	for (const FName& ClaimedThresholdId : Starwell->ReachedStoryUnlockIds)
	{
		if (ClaimedThresholdId == TEXT("DebugReliabilityThreshold"))
		{
			++ClaimedReliabilityThresholdCount;
		}
	}

	if (Starwell->OfferingProgress != 5 || Starwell->TotalFishAccepted != 1 || Starwell->TotalEchoScalesGenerated != 5 || ClaimedReliabilityThresholdCount != 1)
	{
		AddReliabilityFailure(FString::Printf(TEXT("Starwell progress or claimed threshold IDs did not restore correctly. Progress=%d Accepted=%d EchoGenerated=%d ClaimedThresholdCount=%d."),
			Starwell->OfferingProgress,
			Starwell->TotalFishAccepted,
			Starwell->TotalEchoScalesGenerated,
			ClaimedReliabilityThresholdCount));
	}
	else
	{
		RecordSaveLoadReliabilityLog(TEXT("Starwell progress and claimed threshold ID restored correctly."));
	}

	if (ObservedFishingRestoreEvents <= 0 || ObservedEconomyRestoreEvents <= 0 || ObservedDeckRestoreEvents <= 0 || ObservedStarwellRestoreEvents <= 0)
	{
		AddReliabilityFailure(FString::Printf(TEXT("One or more post-load restoration events were not observed by the debug actor. Fishing=%d Economy=%d Deck=%d Starwell=%d"),
			ObservedFishingRestoreEvents,
			ObservedEconomyRestoreEvents,
			ObservedDeckRestoreEvents,
			ObservedStarwellRestoreEvents));
	}
	else
	{
		RecordSaveLoadReliabilityLog(TEXT("Blueprint-facing post-load restoration hooks were mirrored and observed by the debug actor."));
	}

	if (ObservedThresholdReachedEvents != 0)
	{
		AddReliabilityFailure(TEXT("Threshold reached event fired during save restoration."));
	}
	else
	{
		RecordSaveLoadReliabilityLog(TEXT("No one-time threshold event replayed during save restoration."));
	}

	ResetObservedEventCounts();
	const int32 ClaimedThresholdCountBeforeDuplicateCheck = Starwell->ReachedStoryUnlockIds.Num();
	Starwell->AcceptCaughtFish(Fish, EconomyComponent.Get());
	if (ObservedThresholdReachedEvents != 0)
	{
		AddReliabilityFailure(TEXT("Claimed Starwell threshold fired again after loading and accepting another fish."));
	}
	else if (Starwell->ReachedStoryUnlockIds.Num() != ClaimedThresholdCountBeforeDuplicateCheck)
	{
		AddReliabilityFailure(TEXT("Claimed Starwell threshold IDs changed after a duplicate-threshold offering check."));
	}
	else
	{
		RecordSaveLoadReliabilityLog(TEXT("Duplicate threshold reward did not fire after loading and accepting another fish."));
	}

	Starwell->Destroy();
	Result.Summary = Result.bPassed
		? TEXT("Debug Save/Load Reliability validation passed.")
		: FString::Printf(TEXT("Debug Save/Load Reliability validation failed with %d issue(s)."), Result.Failures.Num());
	BroadcastActionResult(Result.bPassed);
	return Result;
}

AStarwell* ANyxGameplayDebugActor::EnsureStarwell()
{
	if (StarwellTarget != nullptr)
	{
		return StarwellTarget.Get();
	}

	if (SpawnedStarwell == nullptr && bSpawnStarwellOnBeginPlay)
	{
		SpawnedStarwell = GetWorld() != nullptr ? GetWorld()->SpawnActor<AStarwell>(AStarwell::StaticClass(), GetActorTransform()) : nullptr;
	}

	return SpawnedStarwell.Get();
}

void ANyxGameplayDebugActor::BroadcastActionResult(bool bSucceeded)
{
	OnDebugActionCompleted.Broadcast(this, bSucceeded);
}

void ANyxGameplayDebugActor::BindRestoreEventObservers()
{
	if (FishingComponent != nullptr)
	{
		FishingComponent->OnFishingStateRestoredNative.RemoveAll(this);
		FishingComponent->OnFishingStateRestoredNative.AddUObject(this, &ANyxGameplayDebugActor::HandleFishingStateRestored);
	}

	if (EconomyComponent != nullptr)
	{
		EconomyComponent->OnEconomySaveAppliedNative.RemoveAll(this);
		EconomyComponent->OnEconomySaveAppliedNative.AddUObject(this, &ANyxGameplayDebugActor::HandleEconomyStateRestored);
	}

	if (DeckComponent != nullptr)
	{
		DeckComponent->OnDeckSaveAppliedNative.RemoveAll(this);
		DeckComponent->OnDeckSaveAppliedNative.AddUObject(this, &ANyxGameplayDebugActor::HandleDeckStateRestored);
	}

	if (AStarwell* Starwell = GetResolvedStarwell())
	{
		Starwell->OnStarwellStateRestoredNative.RemoveAll(this);
		Starwell->OnStarwellStateRestoredNative.AddUObject(this, &ANyxGameplayDebugActor::HandleStarwellStateRestored);
		Starwell->OnOfferingThresholdReachedNative.RemoveAll(this);
		Starwell->OnOfferingThresholdReachedNative.AddUObject(this, &ANyxGameplayDebugActor::HandleStarwellThresholdReached);
	}
}

void ANyxGameplayDebugActor::ResetObservedEventCounts()
{
	ObservedFishingRestoreEvents = 0;
	ObservedEconomyRestoreEvents = 0;
	ObservedDeckRestoreEvents = 0;
	ObservedStarwellRestoreEvents = 0;
	ObservedThresholdReachedEvents = 0;
}

void ANyxGameplayDebugActor::RecordSaveLoadReliabilityLog(const FString& Message)
{
	LastSaveLoadReliabilityValidationLog.Add(Message);
	UE_LOG(LogTemp, Log, TEXT("Nyx Save/Load Reliability: %s"), *Message);
}

void ANyxGameplayDebugActor::HandleFishingStateRestored(UFishingComponent* RestoredFishingComponent)
{
	++ObservedFishingRestoreEvents;
	UE_LOG(LogTemp, Log, TEXT("Nyx debug observed fishing state restored on %s."), *GetNameSafe(RestoredFishingComponent));
}

void ANyxGameplayDebugActor::HandleEconomyStateRestored(UEconomyComponent* RestoredEconomyComponent)
{
	++ObservedEconomyRestoreEvents;
	UE_LOG(LogTemp, Log, TEXT("Nyx debug observed economy state restored on %s."), *GetNameSafe(RestoredEconomyComponent));
}

void ANyxGameplayDebugActor::HandleDeckStateRestored(UDeckComponent* RestoredDeckComponent)
{
	++ObservedDeckRestoreEvents;
	UE_LOG(LogTemp, Log, TEXT("Nyx debug observed deck state restored on %s."), *GetNameSafe(RestoredDeckComponent));
}

void ANyxGameplayDebugActor::HandleStarwellStateRestored(AStarwell* RestoredStarwell)
{
	++ObservedStarwellRestoreEvents;
	UE_LOG(LogTemp, Log, TEXT("Nyx debug observed Starwell state restored on %s."), *GetNameSafe(RestoredStarwell));
}

void ANyxGameplayDebugActor::HandleStarwellThresholdReached(AStarwell* RestoredStarwell, FName StoryUnlockId, int32 RequiredProgress, const FGameplayTagContainer& StoryUnlockTags)
{
	++ObservedThresholdReachedEvents;
	UE_LOG(LogTemp, Log, TEXT("Nyx debug observed Starwell threshold reached on %s: %s at %d."), *GetNameSafe(RestoredStarwell), *StoryUnlockId.ToString(), RequiredProgress);
}
