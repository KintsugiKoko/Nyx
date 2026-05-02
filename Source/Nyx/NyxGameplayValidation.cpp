#include "NyxGameplayValidation.h"

#include "CardDataAsset.h"
#include "DeckComponent.h"
#include "EconomyComponent.h"
#include "Engine/World.h"
#include "FishDataAsset.h"
#include "FishingComponent.h"
#include "Kismet/GameplayStatics.h"
#include "NyxSaveGame.h"
#include "Starwell.h"

#if WITH_DEV_AUTOMATION_TESTS
#include "Engine/Engine.h"
#include "Misc/AutomationTest.h"
#include "NyxGameplayDebugActor.h"
#endif

namespace
{
UCardDataAsset* MakeTransientCard(const TCHAR* Name)
{
	UCardDataAsset* Card = NewObject<UCardDataAsset>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UCardDataAsset::StaticClass(), FName(Name)));
	Card->CardName = FText::FromString(FString(Name));
	Card->Cost = 1;
	return Card;
}

UFishDataAsset* MakeTransientFish(const TCHAR* Name, int32 BaseValue)
{
	UFishDataAsset* Fish = NewObject<UFishDataAsset>(GetTransientPackage(), MakeUniqueObjectName(GetTransientPackage(), UFishDataAsset::StaticClass(), FName(Name)));
	Fish->FishId = FName(Name);
	Fish->DisplayName = FText::FromString(FString(Name));
	Fish->BaseStardustValue = BaseValue;
	Fish->BiteTimeRange = FVector2D(0.0f, 0.0f);
	Fish->ReelDifficulty = 1.0f;
	return Fish;
}

UWorld* ResolveWorld(UObject* WorldContextObject)
{
	return WorldContextObject != nullptr ? WorldContextObject->GetWorld() : nullptr;
}

#if WITH_DEV_AUTOMATION_TESTS
UWorld* CreateNyxAutomationWorld()
{
	FWorldInitializationValues InitValues;
	InitValues.InitializeScenes(false)
		.AllowAudioPlayback(false)
		.RequiresHitProxies(false)
		.CreatePhysicsScene(false)
		.CreateNavigation(false)
		.CreateAISystem(false)
		.ShouldSimulatePhysics(false)
		.SetTransactional(false)
		.CreateFXSystem(false);

	UWorld* World = UWorld::CreateWorld(
		EWorldType::Game,
		false,
		MakeUniqueObjectName(GetTransientPackage(), UWorld::StaticClass(), TEXT("NyxAutomationWorld")),
		GetTransientPackage(),
		true,
		ERHIFeatureLevel::Num,
		&InitValues);

	if (World != nullptr && GEngine != nullptr)
	{
		FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
		WorldContext.SetCurrentWorld(World);
	}

	return World;
}

void DestroyNyxAutomationWorld(UWorld* World)
{
	if (World != nullptr)
	{
		if (GEngine != nullptr)
		{
			GEngine->DestroyWorldContext(World);
		}

		World->DestroyWorld(false);
	}
}
#endif
}

FNyxGameplayValidationResult UNyxGameplayValidationLibrary::ValidateEconomySystem()
{
	FNyxGameplayValidationResult Result;

	UEconomyComponent* Economy = NewObject<UEconomyComponent>(GetTransientPackage());
	Economy->AddResource(ENyxResourceType::Stardust, 25);
	Economy->AddResource(ENyxResourceType::MoonPearls, 3);

	if (Economy->Stardust != 25)
	{
		Result.AddFailure(TEXT("Stardust did not increase to 25 after AddResource."));
	}

	if (!Economy->SpendResource(ENyxResourceType::Stardust, 10) || Economy->Stardust != 15)
	{
		Result.AddFailure(TEXT("SpendResource did not spend 10 Stardust correctly."));
	}

	if (Economy->SpendResource(ENyxResourceType::MoonPearls, 99))
	{
		Result.AddFailure(TEXT("SpendResource allowed spending more Moon Pearls than available."));
	}

	UUpgradeDataAsset* Upgrade = NewObject<UUpgradeDataAsset>(GetTransientPackage());
	Upgrade->UpgradeId = TEXT("ValidationUpgrade");

	FNyxResourceAmount StardustCostA;
	StardustCostA.ResourceType = ENyxResourceType::Stardust;
	StardustCostA.Amount = 10;

	FNyxResourceAmount StardustCostB;
	StardustCostB.ResourceType = ENyxResourceType::Stardust;
	StardustCostB.Amount = 5;

	FNyxResourceAmount MoonPearlCost;
	MoonPearlCost.ResourceType = ENyxResourceType::MoonPearls;
	MoonPearlCost.Amount = 2;

	Upgrade->Costs.Add(StardustCostA);
	Upgrade->Costs.Add(StardustCostB);
	Upgrade->Costs.Add(MoonPearlCost);

	if (!Economy->CanAfford(Upgrade))
	{
		Result.AddFailure(TEXT("CanAfford failed for an affordable upgrade with duplicate Stardust cost rows."));
	}

	if (!Economy->ApplyUpgrade(Upgrade))
	{
		Result.AddFailure(TEXT("ApplyUpgrade failed for an affordable upgrade."));
	}

	if (Economy->Stardust != 0 || Economy->MoonPearls != 1 || Economy->GetUpgradeApplyCount(Upgrade) != 1)
	{
		Result.AddFailure(TEXT("ApplyUpgrade did not spend resources or track apply count correctly."));
	}

	FinalizeValidationResult(Result, TEXT("Economy"));
	return Result;
}

FNyxGameplayValidationResult UNyxGameplayValidationLibrary::ValidateDeckSystem()
{
	FNyxGameplayValidationResult Result;

	UDeckComponent* Deck = NewObject<UDeckComponent>(GetTransientPackage());
	UCardDataAsset* FirstCard = MakeTransientCard(TEXT("ValidationCardA"));
	UCardDataAsset* SecondCard = MakeTransientCard(TEXT("ValidationCardB"));

	Deck->DrawPile.Add(FirstCard);
	Deck->DrawPile.Add(SecondCard);

	UCardDataAsset* DrawnCard = Deck->DrawCard();
	if (DrawnCard != FirstCard || Deck->GetDrawPileCount() != 1 || Deck->GetHandCount() != 1)
	{
		Result.AddFailure(TEXT("DrawCard did not move the top draw-pile card into hand."));
	}

	if (!Deck->PlayCard(DrawnCard) || Deck->GetHandCount() != 0 || Deck->GetDiscardPileCount() != 1)
	{
		Result.AddFailure(TEXT("PlayCard did not move the card from hand to discard."));
	}

	Deck->DrawCard();
	Deck->PlayCard(SecondCard);
	Deck->ShuffleDiscardIntoDeck();

	if (Deck->GetDrawPileCount() != 2 || Deck->GetDiscardPileCount() != 0 || Deck->ShuffleCount != 1)
	{
		Result.AddFailure(TEXT("ShuffleDiscardIntoDeck did not move discard into draw pile and increment ShuffleCount."));
	}

	FinalizeValidationResult(Result, TEXT("Deck"));
	return Result;
}

FNyxGameplayValidationResult UNyxGameplayValidationLibrary::ValidateFishingSystem()
{
	FNyxGameplayValidationResult Result;

	UFishingComponent* Fishing = NewObject<UFishingComponent>(GetTransientPackage());
	UFishDataAsset* Fish = MakeTransientFish(TEXT("ValidationFish"), 4);
	Fishing->AvailableFish.Add(Fish);
	Fishing->RandomSeed = 10;
	Fishing->CastIndex = 0;
	Fishing->bOfferCompletedCatchesToStarwell = false;

	if (!Fishing->StartCast())
	{
		Result.AddFailure(TEXT("StartCast failed with one available fish."));
	}

	if (Fishing->SelectedFish != Fish || Fishing->FishingState != EFishingState::Casting || Fishing->CastIndex != 1)
	{
		Result.AddFailure(TEXT("StartCast did not select the fish and increment CastIndex."));
	}

	// This validation runs without a world, so timer-driven bites are advanced manually.
	Fishing->FishingState = EFishingState::FishBiting;

	if (!Fishing->StartReel() || Fishing->FishingState != EFishingState::Reeling)
	{
		Result.AddFailure(TEXT("StartReel failed after fish bite."));
	}

	Fishing->CompleteCatch();
	if (Fishing->FishingState != EFishingState::CatchComplete || Fishing->SelectedFish != nullptr)
	{
		Result.AddFailure(TEXT("CompleteCatch did not complete and clear SelectedFish."));
	}

	FinalizeValidationResult(Result, TEXT("Fishing"));
	return Result;
}

FNyxGameplayValidationResult UNyxGameplayValidationLibrary::ValidateStarwellSystem(UObject* WorldContextObject)
{
	FNyxGameplayValidationResult Result;

	UWorld* World = ResolveWorld(WorldContextObject);
	if (World == nullptr)
	{
		Result.AddFailure(TEXT("Starwell validation requires a valid WorldContextObject, such as an actor in PIE."));
		FinalizeValidationResult(Result, TEXT("Starwell"));
		return Result;
	}

	AStarwell* Starwell = World->SpawnActor<AStarwell>(AStarwell::StaticClass());
	UEconomyComponent* Economy = NewObject<UEconomyComponent>(GetTransientPackage());
	UFishDataAsset* Fish = MakeTransientFish(TEXT("ValidationStarwellFish"), 5);

	if (Starwell == nullptr)
	{
		Result.AddFailure(TEXT("Failed to spawn transient Starwell."));
		FinalizeValidationResult(Result, TEXT("Starwell"));
		return Result;
	}

	FStarwellOfferingThreshold Threshold;
	Threshold.RequiredProgress = 5;
	Threshold.StoryUnlockId = TEXT("ValidationStory");
	Starwell->OfferingThresholds.Add(Threshold);

	TArray<FString> ThresholdValidationFailures;
	if (!Starwell->ValidateOfferingThresholds(ThresholdValidationFailures))
	{
		Result.AddFailure(TEXT("A valid Starwell threshold definition failed validation."));
	}

	const int32 EchoScalesGranted = Starwell->AcceptCaughtFish(Fish, Economy);
	if (EchoScalesGranted != 5 || Economy->EchoScales != 5)
	{
		Result.AddFailure(TEXT("AcceptCaughtFish did not convert fish value into Echo Scales."));
	}

	if (Starwell->OfferingProgress != 5 || !Starwell->HasReachedStoryUnlock(TEXT("ValidationStory")))
	{
		Result.AddFailure(TEXT("Starwell did not track offering progress or reached story threshold."));
	}

	FStarwellOfferingThreshold MissingIdThreshold;
	MissingIdThreshold.RequiredProgress = 10;
	Starwell->OfferingThresholds.Add(MissingIdThreshold);
	if (Starwell->ValidateOfferingThresholds(ThresholdValidationFailures))
	{
		Result.AddFailure(TEXT("Starwell threshold validation allowed a threshold with no StoryUnlockId."));
	}
	Starwell->OfferingThresholds.RemoveAt(Starwell->OfferingThresholds.Num() - 1);

	FStarwellOfferingThreshold DuplicateIdThreshold;
	DuplicateIdThreshold.RequiredProgress = 12;
	DuplicateIdThreshold.StoryUnlockId = TEXT("ValidationStory");
	Starwell->OfferingThresholds.Add(DuplicateIdThreshold);
	if (Starwell->ValidateOfferingThresholds(ThresholdValidationFailures))
	{
		Result.AddFailure(TEXT("Starwell threshold validation allowed duplicate StoryUnlockId values."));
	}

	Starwell->Destroy();

	FinalizeValidationResult(Result, TEXT("Starwell"));
	return Result;
}

FNyxGameplayValidationResult UNyxGameplayValidationLibrary::ValidateSaveGameSystem()
{
	FNyxGameplayValidationResult Result;

	UFishingComponent* Fishing = NewObject<UFishingComponent>(GetTransientPackage());
	UEconomyComponent* Economy = NewObject<UEconomyComponent>(GetTransientPackage());
	UDeckComponent* Deck = NewObject<UDeckComponent>(GetTransientPackage());

	UFishDataAsset* Fish = MakeTransientFish(TEXT("ValidationSavedFish"), 6);
	UCardDataAsset* Card = MakeTransientCard(TEXT("ValidationSavedCard"));

	Fishing->AvailableFish.Add(Fish);
	Fishing->SelectedFish = Fish;
	Fishing->FishingState = EFishingState::Casting;
	Fishing->RandomSeed = 77;
	Fishing->CastIndex = 3;
	Fishing->Tension = 0.5f;
	Fishing->CurrentBiteTime = 12.0f;
	Fishing->RecordCatchProgress(Fish, true);
	Economy->AddResource(ENyxResourceType::Stardust, 42);
	Economy->AddResource(ENyxResourceType::EchoScales, 8);
	Deck->DrawPile.Add(Card);
	Deck->ShuffleSeed = 99;

	UNyxSaveGame* SaveGame = UNyxSaveGameLibrary::CaptureNyxSaveGame(Fishing, Economy, Deck, nullptr);
	if (SaveGame == nullptr)
	{
		Result.AddFailure(TEXT("CaptureNyxSaveGame returned null."));
		FinalizeValidationResult(Result, TEXT("SaveGame"));
		return Result;
	}

	const FString ValidationSlotName = TEXT("NyxValidationSlot");
	const int32 ValidationUserIndex = 0;
	UGameplayStatics::DeleteGameInSlot(ValidationSlotName, ValidationUserIndex);

	if (!UNyxSaveGameLibrary::SaveNyxGameToSlot(ValidationSlotName, ValidationUserIndex, Fishing, Economy, Deck, nullptr))
	{
		Result.AddFailure(TEXT("SaveNyxGameToSlot failed during disk round-trip validation."));
	}
	else
	{
		UNyxSaveGame* LoadedSaveGame = UNyxSaveGameLibrary::LoadNyxGameFromSlot(ValidationSlotName, ValidationUserIndex);
		if (LoadedSaveGame == nullptr)
		{
			Result.AddFailure(TEXT("LoadNyxGameFromSlot returned null during disk round-trip validation."));
		}
		else if (LoadedSaveGame->Fishing.RandomSeed != 77 || LoadedSaveGame->Fishing.CastIndex != 3 || LoadedSaveGame->Fishing.FishingState != EFishingState::Idle || LoadedSaveGame->Fishing.Tension != 0.0f || LoadedSaveGame->Fishing.CurrentBiteTime != 0.0f || LoadedSaveGame->Fishing.Progress.TotalCatches != 1 || LoadedSaveGame->Fishing.Progress.TotalPerfectCatches != 1 || LoadedSaveGame->Economy.Stardust != 42 || LoadedSaveGame->Economy.EchoScales != 8 || LoadedSaveGame->Deck.ShuffleSeed != 99)
		{
			Result.AddFailure(TEXT("Disk round-trip save validation did not preserve primitive gameplay state."));
		}
	}

	UGameplayStatics::DeleteGameInSlot(ValidationSlotName, ValidationUserIndex);

	Fishing->AvailableFish.Reset();
	Fishing->SelectedFish = nullptr;
	Fishing->FishingState = EFishingState::Idle;
	Fishing->RandomSeed = 0;
	Fishing->CastIndex = 0;
	Fishing->Tension = 0.0f;
	Fishing->CurrentBiteTime = 0.0f;
	Fishing->FishingProgress = FNyxFishingProgressData();
	Economy->Stardust = 0;
	Economy->EchoScales = 0;
	Deck->DrawPile.Reset();
	Deck->ShuffleSeed = 0;

	if (!UNyxSaveGameLibrary::ApplyNyxSaveGame(SaveGame, Fishing, Economy, Deck, nullptr))
	{
		Result.AddFailure(TEXT("ApplyNyxSaveGame failed."));
	}

	if (Fishing->AvailableFish.Num() != 1 || Fishing->RandomSeed != 77 || Fishing->CastIndex != 3)
	{
		Result.AddFailure(TEXT("Fishing state did not restore from save."));
	}

	if (Fishing->FishingState != EFishingState::Idle || Fishing->SelectedFish != nullptr || Fishing->CurrentBiteTime != 0.0f)
	{
		Result.AddFailure(TEXT("Active fishing state was not normalized to Idle after applying save data."));
	}

	if (!Fishing->HasDiscoveredFish(TEXT("ValidationSavedFish")) || Fishing->GetCatchCountForFish(TEXT("ValidationSavedFish")) != 1 || Fishing->GetPerfectCatchCountForFish(TEXT("ValidationSavedFish")) != 1)
	{
		Result.AddFailure(TEXT("Durable fishing collection progress did not restore from save."));
	}

	if (Economy->Stardust != 42 || Economy->EchoScales != 8)
	{
		Result.AddFailure(TEXT("Economy resources did not restore from save."));
	}

	if (Deck->DrawPile.Num() != 1 || Deck->ShuffleSeed != 99)
	{
		Result.AddFailure(TEXT("Deck state did not restore from save."));
	}

	FinalizeValidationResult(Result, TEXT("SaveGame"));
	return Result;
}

FNyxGameplayValidationResult UNyxGameplayValidationLibrary::RunAllGameplayValidations(UObject* WorldContextObject)
{
	FNyxGameplayValidationResult Result;
	Result.Summary = TEXT("Nyx gameplay validation suite");

	MergeValidationResult(Result, ValidateEconomySystem());
	MergeValidationResult(Result, ValidateDeckSystem());
	MergeValidationResult(Result, ValidateFishingSystem());
	MergeValidationResult(Result, ValidateSaveGameSystem());
	MergeValidationResult(Result, ValidateStarwellSystem(WorldContextObject));

	FinalizeValidationResult(Result, TEXT("All Gameplay"));
	return Result;
}

void UNyxGameplayValidationLibrary::MergeValidationResult(FNyxGameplayValidationResult& Target, const FNyxGameplayValidationResult& Source)
{
	if (!Source.bPassed)
	{
		Target.bPassed = false;
	}

	for (const FString& Failure : Source.Failures)
	{
		Target.Failures.Add(Failure);
	}
}

void UNyxGameplayValidationLibrary::FinalizeValidationResult(FNyxGameplayValidationResult& Result, const FString& SystemName)
{
	Result.Summary = Result.bPassed
		? FString::Printf(TEXT("%s validation passed."), *SystemName)
		: FString::Printf(TEXT("%s validation failed with %d issue(s)."), *SystemName, Result.Failures.Num());
}

#if WITH_DEV_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNyxEconomyValidationAutomationTest, "Nyx.Gameplay.Economy", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FNyxEconomyValidationAutomationTest::RunTest(const FString& Parameters)
{
	const FNyxGameplayValidationResult Result = UNyxGameplayValidationLibrary::ValidateEconomySystem();
	for (const FString& Failure : Result.Failures)
	{
		AddError(Failure);
	}
	return Result.bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNyxDeckValidationAutomationTest, "Nyx.Gameplay.Deck", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FNyxDeckValidationAutomationTest::RunTest(const FString& Parameters)
{
	const FNyxGameplayValidationResult Result = UNyxGameplayValidationLibrary::ValidateDeckSystem();
	for (const FString& Failure : Result.Failures)
	{
		AddError(Failure);
	}
	return Result.bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNyxFishingValidationAutomationTest, "Nyx.Gameplay.Fishing", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FNyxFishingValidationAutomationTest::RunTest(const FString& Parameters)
{
	const FNyxGameplayValidationResult Result = UNyxGameplayValidationLibrary::ValidateFishingSystem();
	for (const FString& Failure : Result.Failures)
	{
		AddError(Failure);
	}
	return Result.bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNyxSaveGameValidationAutomationTest, "Nyx.Gameplay.SaveGame", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FNyxSaveGameValidationAutomationTest::RunTest(const FString& Parameters)
{
	const FNyxGameplayValidationResult Result = UNyxGameplayValidationLibrary::ValidateSaveGameSystem();
	for (const FString& Failure : Result.Failures)
	{
		AddError(Failure);
	}
	return Result.bPassed;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FNyxDebugSaveLoadReliabilityAutomationTest, "Nyx.Gameplay.DebugSaveLoadReliability", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FNyxDebugSaveLoadReliabilityAutomationTest::RunTest(const FString& Parameters)
{
	UWorld* World = CreateNyxAutomationWorld();
	if (World == nullptr)
	{
		AddError(TEXT("Failed to create transient automation world for debug save/load reliability test."));
		return false;
	}

	ANyxGameplayDebugActor* DebugActor = World->SpawnActor<ANyxGameplayDebugActor>(ANyxGameplayDebugActor::StaticClass());
	if (DebugActor == nullptr)
	{
		AddError(TEXT("Failed to spawn ANyxGameplayDebugActor in transient automation world."));
		DestroyNyxAutomationWorld(World);
		return false;
	}

	const FNyxGameplayValidationResult Result = DebugActor->DebugValidateSaveLoadReliability();
	for (const FString& Failure : Result.Failures)
	{
		AddError(Failure);
	}

	DebugActor->Destroy();
	DestroyNyxAutomationWorld(World);

	return Result.bPassed;
}

#endif
