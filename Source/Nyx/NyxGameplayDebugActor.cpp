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

	if (FishingComponent == nullptr || EconomyComponent == nullptr || DeckComponent == nullptr)
	{
		Result.AddFailure(TEXT("Debug actor is missing one or more gameplay components."));
		BroadcastActionResult(false);
		return Result;
	}

	AStarwell* Starwell = GetWorld() != nullptr ? GetWorld()->SpawnActor<AStarwell>(AStarwell::StaticClass(), GetActorTransform()) : nullptr;
	if (Starwell == nullptr)
	{
		Result.AddFailure(TEXT("Debug actor could not spawn an isolated Starwell for validation."));
		BroadcastActionResult(false);
		return Result;
	}

	BindRestoreEventObservers();
	Starwell->OnStarwellStateRestored.AddUniqueDynamic(this, &ANyxGameplayDebugActor::HandleStarwellStateRestored);
	Starwell->OnOfferingThresholdReached.AddUniqueDynamic(this, &ANyxGameplayDebugActor::HandleStarwellThresholdReached);
	UE_LOG(LogTemp, Log, TEXT("Nyx debug binding probe: HasFishingFunction=%d FishingBound=%d EconomyBound=%d DeckBound=%d StarwellBound=%d."),
		FindFunction(TEXT("HandleFishingStateRestored")) != nullptr,
		FishingComponent->OnFishingStateRestored.IsBound(),
		EconomyComponent->OnEconomySaveApplied.IsBound(),
		DeckComponent->OnDeckSaveApplied.IsBound(),
		Starwell->OnStarwellStateRestored.IsBound());

	FishingComponent->OnFishingStateRestored.Broadcast(FishingComponent.Get());
	EconomyComponent->OnEconomySaveApplied.Broadcast(EconomyComponent.Get());
	DeckComponent->OnDeckSaveApplied.Broadcast(DeckComponent.Get());
	Starwell->OnStarwellStateRestored.Broadcast(Starwell);
	UE_LOG(LogTemp, Log, TEXT("Nyx debug restore event counts after binding probe: Fishing=%d Economy=%d Deck=%d Starwell=%d."),
		ObservedFishingRestoreEvents,
		ObservedEconomyRestoreEvents,
		ObservedDeckRestoreEvents,
		ObservedStarwellRestoreEvents);
	ResetObservedEventCounts();

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
		Result.AddFailure(TEXT("Debug reliability setup did not reach the expected Starwell threshold."));
	}

	UNyxSaveGame* SaveGame = UNyxSaveGameLibrary::CaptureNyxSaveGame(FishingComponent.Get(), EconomyComponent.Get(), DeckComponent.Get(), Starwell);
	if (SaveGame == nullptr)
	{
		Result.AddFailure(TEXT("Debug reliability capture failed."));
		Starwell->Destroy();
		BroadcastActionResult(false);
		return Result;
	}

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
		Result.AddFailure(TEXT("Debug reliability apply failed."));
	}

	UE_LOG(LogTemp, Log, TEXT("Nyx debug restore event counts after apply: Fishing=%d Economy=%d Deck=%d Starwell=%d Threshold=%d."),
		ObservedFishingRestoreEvents,
		ObservedEconomyRestoreEvents,
		ObservedDeckRestoreEvents,
		ObservedStarwellRestoreEvents,
		ObservedThresholdReachedEvents);

	if (FishingComponent->FishingState != EFishingState::Idle || FishingComponent->SelectedFish != nullptr || FishingComponent->CurrentBiteTime != 0.0f || !FMath::IsNearlyZero(FishingComponent->Tension))
	{
		Result.AddFailure(TEXT("Active fishing runtime state did not restore to a clean Idle state."));
	}

	if (!FishingComponent->HasDiscoveredFish(TEXT("DebugReliabilityGlowMinnow")) || FishingComponent->GetCatchCountForFish(TEXT("DebugReliabilityGlowMinnow")) != 1 || FishingComponent->GetPerfectCatchCountForFish(TEXT("DebugReliabilityGlowMinnow")) != 1)
	{
		Result.AddFailure(TEXT("Durable fishing collection progress did not restore correctly."));
	}

	if (Starwell->OfferingProgress != 5 || Starwell->TotalFishAccepted != 1 || !Starwell->HasReachedStoryUnlock(TEXT("DebugReliabilityThreshold")))
	{
		Result.AddFailure(TEXT("Starwell progress or claimed threshold IDs did not restore correctly."));
	}

	if (ObservedFishingRestoreEvents <= 0 || ObservedEconomyRestoreEvents <= 0 || ObservedDeckRestoreEvents <= 0 || ObservedStarwellRestoreEvents <= 0)
	{
		Result.AddFailure(FString::Printf(TEXT("One or more post-load restoration events were not observed by the debug actor. Fishing=%d Economy=%d Deck=%d Starwell=%d"),
			ObservedFishingRestoreEvents,
			ObservedEconomyRestoreEvents,
			ObservedDeckRestoreEvents,
			ObservedStarwellRestoreEvents));
	}

	if (ObservedThresholdReachedEvents != 0)
	{
		Result.AddFailure(TEXT("Threshold reached event fired during save restoration."));
	}

	ResetObservedEventCounts();
	Starwell->AcceptCaughtFish(Fish, EconomyComponent.Get());
	if (ObservedThresholdReachedEvents != 0)
	{
		Result.AddFailure(TEXT("Claimed Starwell threshold fired again after loading and accepting another fish."));
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
		FishingComponent->OnFishingStateRestored.AddUniqueDynamic(this, &ANyxGameplayDebugActor::HandleFishingStateRestored);
	}

	if (EconomyComponent != nullptr)
	{
		EconomyComponent->OnEconomySaveApplied.AddUniqueDynamic(this, &ANyxGameplayDebugActor::HandleEconomyStateRestored);
	}

	if (DeckComponent != nullptr)
	{
		DeckComponent->OnDeckSaveApplied.AddUniqueDynamic(this, &ANyxGameplayDebugActor::HandleDeckStateRestored);
	}

	if (AStarwell* Starwell = GetResolvedStarwell())
	{
		Starwell->OnStarwellStateRestored.AddUniqueDynamic(this, &ANyxGameplayDebugActor::HandleStarwellStateRestored);
		Starwell->OnOfferingThresholdReached.AddUniqueDynamic(this, &ANyxGameplayDebugActor::HandleStarwellThresholdReached);
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
