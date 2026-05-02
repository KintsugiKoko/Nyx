#include "FishingComponent.h"

#include "Engine/World.h"
#include "EconomyComponent.h"
#include "FishDataAsset.h"
#include "GameFramework/Actor.h"
#include "Starwell.h"
#include "TimerManager.h"

UFishingComponent::UFishingComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	RandomSeed = 1337;
	CastIndex = 0;
	SelectedFish = nullptr;
	FishingState = EFishingState::Idle;
	Tension = 0.0f;
	CurrentBiteTime = 0.0f;
	FishingPowerMultiplier = 1.0f;
	BiteTimeMultiplier = 1.0f;
	BonusFishPullsOnCatch = 0;
	bOfferCompletedCatchesToStarwell = true;
	StarwellTarget = nullptr;
	EconomyTarget = nullptr;
}

bool UFishingComponent::StartCast()
{
	if (FishingState == EFishingState::Casting || FishingState == EFishingState::FishBiting || FishingState == EFishingState::Reeling)
	{
		return false;
	}

	FRandomStream RandomStream = MakeCastRandomStream();
	SelectedFish = SelectFishForCast(RandomStream);

	if (SelectedFish == nullptr)
	{
		SetFishingState(EFishingState::CatchFailed);
		OnCatchFailed.Broadcast(this, nullptr);
		return false;
	}

	CurrentBiteTime = GenerateBiteTimeForFish(SelectedFish, RandomStream);
	SetTension(0.0f);
	SetFishingState(EFishingState::Casting);

	// Advance only after the cast data is generated so RandomSeed + CastIndex can recreate this cast.
	++CastIndex;
	OnCastStarted.Broadcast(this);

	if (UWorld* World = GetWorld())
	{
		if (CurrentBiteTime <= 0.0f)
		{
			HandleFishBite();
		}
		else
		{
			World->GetTimerManager().SetTimer(BiteTimerHandle, this, &UFishingComponent::HandleFishBite, CurrentBiteTime, false);
		}
	}

	return true;
}

void UFishingComponent::CancelCast()
{
	if (!IsFishing())
	{
		return;
	}

	ClearBiteTimer();
	SelectedFish = nullptr;
	CurrentBiteTime = 0.0f;
	SetTension(0.0f);
	SetFishingState(EFishingState::Idle);
	OnCastCancelled.Broadcast(this);
}

bool UFishingComponent::StartReel()
{
	if (FishingState != EFishingState::FishBiting || SelectedFish == nullptr)
	{
		return false;
	}

	ClearBiteTimer();
	SetFishingState(EFishingState::Reeling);
	SetTension(FMath::Clamp((SelectedFish->ReelDifficulty * 0.1f) / FMath::Max(0.1f, FishingPowerMultiplier), 0.0f, 1.0f));
	OnReelStarted.Broadcast(this, SelectedFish);
	return true;
}

void UFishingComponent::CompleteCatch(bool bPerfectCatch)
{
	if (FishingState != EFishingState::Reeling || SelectedFish == nullptr)
	{
		return;
	}

	UFishDataAsset* CaughtFish = SelectedFish;
	ClearBiteTimer();
	SelectedFish = nullptr;
	CurrentBiteTime = 0.0f;
	SetTension(0.0f);
	SetFishingState(EFishingState::CatchComplete);
	const int32 TotalPulls = FMath::Max(1, 1 + BonusFishPullsOnCatch);
	for (int32 PullIndex = 0; PullIndex < TotalPulls; ++PullIndex)
	{
		RecordCatchProgress(CaughtFish, bPerfectCatch);
		ProcessCompletedCatchRewards(CaughtFish);
		if (PullIndex > 0)
		{
			OnBonusFishPullResolved.Broadcast(this, CaughtFish, PullIndex, TotalPulls);
		}
	}
	OnCatchCompleted.Broadcast(this, CaughtFish);
}

void UFishingComponent::FailCatch()
{
	if (!IsFishing() && SelectedFish == nullptr)
	{
		return;
	}

	UFishDataAsset* FailedFish = SelectedFish;
	ClearBiteTimer();
	SelectedFish = nullptr;
	CurrentBiteTime = 0.0f;
	SetTension(0.0f);
	SetFishingState(EFishingState::CatchFailed);
	OnCatchFailed.Broadcast(this, FailedFish);
}

void UFishingComponent::SetTension(float NewTension)
{
	const float ClampedTension = FMath::Clamp(NewTension, 0.0f, 1.0f);

	if (FMath::IsNearlyEqual(Tension, ClampedTension))
	{
		return;
	}

	Tension = ClampedTension;
	OnTensionChanged.Broadcast(this, Tension);
}

void UFishingComponent::SetFishingSkillModifiers(float NewFishingPowerMultiplier, float NewBiteTimeMultiplier, int32 NewBonusFishPullsOnCatch)
{
	FishingPowerMultiplier = FMath::Max(0.1f, NewFishingPowerMultiplier);
	BiteTimeMultiplier = FMath::Clamp(NewBiteTimeMultiplier, 0.1f, 5.0f);
	BonusFishPullsOnCatch = FMath::Max(0, NewBonusFishPullsOnCatch);
}

void UFishingComponent::RestoreSavedState(const TArray<UFishDataAsset*>& RestoredAvailableFish, int32 RestoredRandomSeed, int32 RestoredCastIndex, bool bRestoredOfferCompletedCatchesToStarwell, const FNyxFishingProgressData& RestoredProgress)
{
	ClearTransientRuntimeState();

	AvailableFish.Reset();
	AvailableFish.Reserve(RestoredAvailableFish.Num());
	for (UFishDataAsset* Fish : RestoredAvailableFish)
	{
		if (Fish != nullptr)
		{
			AvailableFish.Add(Fish);
		}
	}

	RandomSeed = RestoredRandomSeed;
	CastIndex = FMath::Max(0, RestoredCastIndex);
	bOfferCompletedCatchesToStarwell = bRestoredOfferCompletedCatchesToStarwell;
	FishingProgress = RestoredProgress;
	FishingProgress.TotalCatches = FMath::Max(0, FishingProgress.TotalCatches);
	FishingProgress.TotalPerfectCatches = FMath::Clamp(FishingProgress.TotalPerfectCatches, 0, FishingProgress.TotalCatches);

	for (int32 Index = FishingProgress.FishCollection.Num() - 1; Index >= 0; --Index)
	{
		FNyxFishCollectionEntry& Entry = FishingProgress.FishCollection[Index];
		if (Entry.FishId.IsNone())
		{
			FishingProgress.FishCollection.RemoveAt(Index);
			continue;
		}

		Entry.CatchCount = FMath::Max(0, Entry.CatchCount);
		Entry.PerfectCatchCount = FMath::Clamp(Entry.PerfectCatchCount, 0, Entry.CatchCount);
	}

	// Runtime cast state, timers, bite windows, and partial catch results are not durable save data.
	SetFishingState(EFishingState::Idle);
	OnFishingSaveApplied.Broadcast(this);
	OnFishingStateRestored.Broadcast(this);
	OnFishingStateRestoredNative.Broadcast(this);
}

void UFishingComponent::RecordCatchProgress(UFishDataAsset* Fish, bool bPerfectCatch)
{
	const FName FishId = ResolveFishProgressId(Fish);
	if (FishId.IsNone())
	{
		return;
	}

	FNyxFishCollectionEntry* Entry = FindMutableCollectionEntry(FishId);
	if (Entry == nullptr)
	{
		Entry = &FishingProgress.FishCollection.AddDefaulted_GetRef();
		Entry->FishId = FishId;
	}

	Entry->CatchCount = FMath::Max(0, Entry->CatchCount) + 1;
	++FishingProgress.TotalCatches;

	if (bPerfectCatch)
	{
		Entry->PerfectCatchCount = FMath::Max(0, Entry->PerfectCatchCount) + 1;
		++FishingProgress.TotalPerfectCatches;
	}

	OnFishingProgressChanged.Broadcast(this, FishId);
}

bool UFishingComponent::HasDiscoveredFish(FName FishId) const
{
	return FindCollectionEntry(FishId) != nullptr;
}

int32 UFishingComponent::GetCatchCountForFish(FName FishId) const
{
	if (const FNyxFishCollectionEntry* Entry = FindCollectionEntry(FishId))
	{
		return Entry->CatchCount;
	}

	return 0;
}

int32 UFishingComponent::GetPerfectCatchCountForFish(FName FishId) const
{
	if (const FNyxFishCollectionEntry* Entry = FindCollectionEntry(FishId))
	{
		return Entry->PerfectCatchCount;
	}

	return 0;
}

void UFishingComponent::SetCatchRewardTargets(AStarwell* NewStarwellTarget, UEconomyComponent* NewEconomyTarget)
{
	StarwellTarget = NewStarwellTarget;
	EconomyTarget = NewEconomyTarget;
}

int32 UFishingComponent::OfferFishToStarwell(UFishDataAsset* Fish)
{
	if (Fish == nullptr || StarwellTarget == nullptr)
	{
		return 0;
	}

	UEconomyComponent* ResolvedEconomyTarget = ResolveEconomyTarget();
	const int32 EchoScalesGranted = StarwellTarget->AcceptCaughtFish(Fish, ResolvedEconomyTarget);
	if (EchoScalesGranted > 0)
	{
		OnCatchOfferedToStarwell.Broadcast(this, Fish, StarwellTarget, ResolvedEconomyTarget, EchoScalesGranted);
	}

	return EchoScalesGranted;
}

bool UFishingComponent::IsFishing() const
{
	return FishingState == EFishingState::Casting || FishingState == EFishingState::FishBiting || FishingState == EFishingState::Reeling;
}

void UFishingComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ClearBiteTimer();
	Super::EndPlay(EndPlayReason);
}

FRandomStream UFishingComponent::MakeCastRandomStream() const
{
	return FRandomStream(RandomSeed + CastIndex);
}

UFishDataAsset* UFishingComponent::SelectFishForCast(FRandomStream& RandomStream) const
{
	TArray<UFishDataAsset*> ValidFish;
	ValidFish.Reserve(AvailableFish.Num());

	for (const TObjectPtr<UFishDataAsset>& FishEntry : AvailableFish)
	{
		UFishDataAsset* Fish = FishEntry.Get();

		if (Fish != nullptr)
		{
			ValidFish.Add(Fish);
		}
	}

	if (ValidFish.IsEmpty())
	{
		return nullptr;
	}

	const int32 SelectedIndex = RandomStream.RandRange(0, ValidFish.Num() - 1);
	return ValidFish[SelectedIndex];
}

float UFishingComponent::GenerateBiteTimeForFish(const UFishDataAsset* Fish, FRandomStream& RandomStream) const
{
	if (Fish == nullptr)
	{
		return 0.0f;
	}

	const float MinBiteTime = FMath::Max(0.0f, FMath::Min(Fish->BiteTimeRange.X, Fish->BiteTimeRange.Y));
	const float MaxBiteTime = FMath::Max(MinBiteTime, FMath::Max(Fish->BiteTimeRange.X, Fish->BiteTimeRange.Y));
	return RandomStream.FRandRange(MinBiteTime, MaxBiteTime) * FMath::Clamp(BiteTimeMultiplier, 0.1f, 5.0f);
}

UEconomyComponent* UFishingComponent::ResolveEconomyTarget() const
{
	if (EconomyTarget != nullptr)
	{
		return EconomyTarget;
	}

	if (AActor* Owner = GetOwner())
	{
		return Owner->FindComponentByClass<UEconomyComponent>();
	}

	return nullptr;
}

void UFishingComponent::HandleFishBite()
{
	if (FishingState != EFishingState::Casting || SelectedFish == nullptr)
	{
		return;
	}

	SetFishingState(EFishingState::FishBiting);
	OnFishBite.Broadcast(this, SelectedFish);
}

void UFishingComponent::ProcessCompletedCatchRewards(UFishDataAsset* CaughtFish)
{
	if (!bOfferCompletedCatchesToStarwell)
	{
		return;
	}

	OfferFishToStarwell(CaughtFish);
}

void UFishingComponent::SetFishingState(EFishingState NewState)
{
	if (FishingState == NewState)
	{
		return;
	}

	FishingState = NewState;
	OnFishingStateChanged.Broadcast(this, FishingState);
}

void UFishingComponent::ClearBiteTimer()
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(BiteTimerHandle);
	}
}

void UFishingComponent::ClearTransientRuntimeState()
{
	ClearBiteTimer();
	SelectedFish = nullptr;
	CurrentBiteTime = 0.0f;
	SetTension(0.0f);
}

FName UFishingComponent::ResolveFishProgressId(const UFishDataAsset* Fish) const
{
	return Fish != nullptr ? Fish->GetStableFishId() : NAME_None;
}

FNyxFishCollectionEntry* UFishingComponent::FindMutableCollectionEntry(FName FishId)
{
	if (FishId.IsNone())
	{
		return nullptr;
	}

	return FishingProgress.FishCollection.FindByPredicate([FishId](const FNyxFishCollectionEntry& Entry)
	{
		return Entry.FishId == FishId;
	});
}

const FNyxFishCollectionEntry* UFishingComponent::FindCollectionEntry(FName FishId) const
{
	if (FishId.IsNone())
	{
		return nullptr;
	}

	return FishingProgress.FishCollection.FindByPredicate([FishId](const FNyxFishCollectionEntry& Entry)
	{
		return Entry.FishId == FishId;
	});
}
