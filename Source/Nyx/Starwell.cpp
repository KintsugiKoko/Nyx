#include "Starwell.h"

#include "Components/SceneComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "EconomyComponent.h"
#include "FishDataAsset.h"

AStarwell::AStarwell()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	VisualMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisualMesh"));
	VisualMesh->SetupAttachment(SceneRoot);
	VisualMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	InteractionCollision = CreateDefaultSubobject<USphereComponent>(TEXT("InteractionCollision"));
	InteractionCollision->SetupAttachment(SceneRoot);
	InteractionCollision->InitSphereRadius(140.0f);
	InteractionCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	InteractionCollision->SetCollisionResponseToAllChannels(ECR_Ignore);
	InteractionCollision->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	OfferingPoint = CreateDefaultSubobject<USceneComponent>(TEXT("OfferingPoint"));
	OfferingPoint->SetupAttachment(SceneRoot);
	OfferingPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));

	VfxSpawnPoint = CreateDefaultSubobject<USceneComponent>(TEXT("VfxSpawnPoint"));
	VfxSpawnPoint->SetupAttachment(SceneRoot);
	VfxSpawnPoint->SetRelativeLocation(FVector(0.0f, 0.0f, 100.0f));

	EchoScalesPerBaseStardustValue = 1;
	MinimumEchoScalesPerFish = 1;
	OfferingProgress = 0;
	TotalFishAccepted = 0;
	TotalEchoScalesGenerated = 0;
}

int32 AStarwell::AcceptCaughtFish(UFishDataAsset* Fish, UEconomyComponent* EconomyComponent)
{
	if (Fish == nullptr)
	{
		return 0;
	}

	const int32 EchoScalesGranted = CalculateEchoScalesForFish(Fish);
	if (EchoScalesGranted <= 0)
	{
		return 0;
	}

	if (EconomyComponent != nullptr)
	{
		EconomyComponent->AddResource(ENyxResourceType::EchoScales, EchoScalesGranted);
	}

	++TotalFishAccepted;
	const int64 NewTotalEchoScales = FMath::Clamp<int64>(static_cast<int64>(TotalEchoScalesGenerated) + EchoScalesGranted, 0, MAX_int32);
	TotalEchoScalesGenerated = static_cast<int32>(NewTotalEchoScales);

	AddOfferingProgress(EchoScalesGranted);
	OnFishAccepted.Broadcast(this, Fish, EchoScalesGranted, OfferingProgress, EconomyComponent);
	return EchoScalesGranted;
}

int32 AStarwell::CalculateEchoScalesForFish(UFishDataAsset* Fish) const
{
	if (Fish == nullptr)
	{
		return 0;
	}

	const int64 ConvertedValue = static_cast<int64>(FMath::Max(0, Fish->BaseStardustValue)) * FMath::Max(0, EchoScalesPerBaseStardustValue);
	const int32 ClampedValue = static_cast<int32>(FMath::Clamp<int64>(ConvertedValue, 0, MAX_int32));
	return FMath::Max(FMath::Max(0, MinimumEchoScalesPerFish), ClampedValue);
}

bool AStarwell::HasReachedStoryUnlock(FName StoryUnlockId) const
{
	return !StoryUnlockId.IsNone() && ReachedStoryUnlockIds.Contains(StoryUnlockId);
}

float AStarwell::GetProgressToNextThreshold() const
{
	int32 NextRequiredProgress = MAX_int32;

	for (const FStarwellOfferingThreshold& Threshold : OfferingThresholds)
	{
		const FName ThresholdSaveKey = GetThresholdSaveKey(Threshold);
		if (!ThresholdSaveKey.IsNone() && Threshold.RequiredProgress > OfferingProgress && !ReachedStoryUnlockIds.Contains(ThresholdSaveKey))
		{
			NextRequiredProgress = FMath::Min(NextRequiredProgress, Threshold.RequiredProgress);
		}
	}

	if (NextRequiredProgress == MAX_int32 || NextRequiredProgress <= 0)
	{
		return 1.0f;
	}

	return FMath::Clamp(static_cast<float>(OfferingProgress) / static_cast<float>(NextRequiredProgress), 0.0f, 1.0f);
}

void AStarwell::RestoreSavedProgress(int32 RestoredEchoScalesPerBaseStardustValue, int32 RestoredMinimumEchoScalesPerFish, int32 RestoredOfferingProgress, int32 RestoredTotalFishAccepted, int32 RestoredTotalEchoScalesGenerated, const TArray<FName>& RestoredReachedStoryUnlockIds)
{
	EchoScalesPerBaseStardustValue = FMath::Max(0, RestoredEchoScalesPerBaseStardustValue);
	MinimumEchoScalesPerFish = FMath::Max(0, RestoredMinimumEchoScalesPerFish);
	OfferingProgress = FMath::Max(0, RestoredOfferingProgress);
	TotalFishAccepted = FMath::Max(0, RestoredTotalFishAccepted);
	TotalEchoScalesGenerated = FMath::Max(0, RestoredTotalEchoScalesGenerated);

	ReachedStoryUnlockIds.Reset();
	for (const FName StoryUnlockId : RestoredReachedStoryUnlockIds)
	{
		if (!StoryUnlockId.IsNone())
		{
			ReachedStoryUnlockIds.AddUnique(StoryUnlockId);
		}
	}

	OnOfferingProgressChanged.Broadcast(this, OfferingProgress);
	OnStarwellSaveApplied.Broadcast(this);
	OnStarwellStateRestored.Broadcast(this);
}

bool AStarwell::ValidateOfferingThresholds(TArray<FString>& OutFailures) const
{
	OutFailures.Reset();
	TMap<FName, int32> SeenUnlockIds;

	for (int32 Index = 0; Index < OfferingThresholds.Num(); ++Index)
	{
		const FStarwellOfferingThreshold& Threshold = OfferingThresholds[Index];
		if (Threshold.StoryUnlockId.IsNone())
		{
			OutFailures.Add(FString::Printf(TEXT("OfferingThresholds[%d] is missing StoryUnlockId. Threshold IDs are required for stable save/load."), Index));
			continue;
		}

		if (const int32* FirstIndex = SeenUnlockIds.Find(Threshold.StoryUnlockId))
		{
			OutFailures.Add(FString::Printf(TEXT("OfferingThresholds[%d] duplicates StoryUnlockId '%s' first used by OfferingThresholds[%d]."), Index, *Threshold.StoryUnlockId.ToString(), *FirstIndex));
			continue;
		}

		SeenUnlockIds.Add(Threshold.StoryUnlockId, Index);
	}

	return OutFailures.IsEmpty();
}

void AStarwell::AddOfferingProgress(int32 ProgressDelta)
{
	if (ProgressDelta <= 0)
	{
		return;
	}

	const int64 NewOfferingProgress = FMath::Clamp<int64>(static_cast<int64>(OfferingProgress) + ProgressDelta, 0, MAX_int32);
	OfferingProgress = static_cast<int32>(NewOfferingProgress);
	OnOfferingProgressChanged.Broadcast(this, OfferingProgress);
	CheckOfferingThresholds();
}

void AStarwell::CheckOfferingThresholds()
{
	for (const FStarwellOfferingThreshold& Threshold : GetReachedThresholdsInOrder())
	{
		const FName ThresholdSaveKey = GetThresholdSaveKey(Threshold);
		if (ThresholdSaveKey.IsNone() || ReachedStoryUnlockIds.Contains(ThresholdSaveKey))
		{
			continue;
		}

		ReachedStoryUnlockIds.Add(ThresholdSaveKey);
		OnOfferingThresholdReached.Broadcast(this, ThresholdSaveKey, Threshold.RequiredProgress, Threshold.StoryUnlockTags);
		OnStoryUnlockAvailable.Broadcast(this, ThresholdSaveKey, Threshold.StoryUnlockTags);
	}
}

FName AStarwell::GetThresholdSaveKey(const FStarwellOfferingThreshold& Threshold) const
{
	return Threshold.StoryUnlockId;
}

TArray<FStarwellOfferingThreshold> AStarwell::GetReachedThresholdsInOrder() const
{
	TArray<FStarwellOfferingThreshold> ReachedThresholds;

	for (const FStarwellOfferingThreshold& Threshold : OfferingThresholds)
	{
		if (Threshold.RequiredProgress <= OfferingProgress)
		{
			ReachedThresholds.Add(Threshold);
		}
	}

	ReachedThresholds.Sort([](const FStarwellOfferingThreshold& Left, const FStarwellOfferingThreshold& Right)
	{
		return Left.RequiredProgress < Right.RequiredProgress;
	});

	return ReachedThresholds;
}
