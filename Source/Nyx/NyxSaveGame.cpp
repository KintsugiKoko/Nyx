#include "NyxSaveGame.h"

#include "DeckComponent.h"
#include "EconomyComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Starwell.h"

namespace
{
void CaptureFishArray(const TArray<TObjectPtr<UFishDataAsset>>& Source, TArray<TSoftObjectPtr<UFishDataAsset>>& OutFish)
{
	OutFish.Reset();
	OutFish.Reserve(Source.Num());

	for (const TObjectPtr<UFishDataAsset>& Fish : Source)
	{
		if (Fish != nullptr)
		{
			OutFish.Add(TSoftObjectPtr<UFishDataAsset>(Fish.Get()));
		}
	}
}

void RestoreFishArray(const TArray<TSoftObjectPtr<UFishDataAsset>>& Source, TArray<UFishDataAsset*>& OutFish)
{
	OutFish.Reset();
	OutFish.Reserve(Source.Num());

	for (const TSoftObjectPtr<UFishDataAsset>& FishReference : Source)
	{
		if (UFishDataAsset* Fish = FishReference.LoadSynchronous())
		{
			OutFish.Add(Fish);
		}
	}
}

void CaptureCardArray(const TArray<TObjectPtr<UCardDataAsset>>& Source, TArray<TSoftObjectPtr<UCardDataAsset>>& OutCards)
{
	OutCards.Reset();
	OutCards.Reserve(Source.Num());

	for (const TObjectPtr<UCardDataAsset>& Card : Source)
	{
		if (Card != nullptr)
		{
			OutCards.Add(TSoftObjectPtr<UCardDataAsset>(Card.Get()));
		}
	}
}

void RestoreCardArray(const TArray<TSoftObjectPtr<UCardDataAsset>>& Source, TArray<UCardDataAsset*>& OutCards)
{
	OutCards.Reset();
	OutCards.Reserve(Source.Num());

	for (const TSoftObjectPtr<UCardDataAsset>& CardReference : Source)
	{
		if (UCardDataAsset* Card = CardReference.LoadSynchronous())
		{
			OutCards.Add(Card);
		}
	}
}
}

UNyxSaveGame* UNyxSaveGameLibrary::CreateNyxSaveGame()
{
	return Cast<UNyxSaveGame>(UGameplayStatics::CreateSaveGameObject(UNyxSaveGame::StaticClass()));
}

UNyxSaveGame* UNyxSaveGameLibrary::CaptureNyxSaveGame(UFishingComponent* FishingComponent, UEconomyComponent* EconomyComponent, UDeckComponent* DeckComponent, AStarwell* Starwell)
{
	UNyxSaveGame* SaveGame = CreateNyxSaveGame();
	if (SaveGame == nullptr)
	{
		return nullptr;
	}

	if (FishingComponent != nullptr)
	{
		CaptureFishArray(FishingComponent->AvailableFish, SaveGame->Fishing.AvailableFish);
		SaveGame->Fishing.SelectedFish = TSoftObjectPtr<UFishDataAsset>();
		SaveGame->Fishing.FishingState = EFishingState::Idle;
		SaveGame->Fishing.RandomSeed = FishingComponent->RandomSeed;
		SaveGame->Fishing.CastIndex = FishingComponent->CastIndex;
		SaveGame->Fishing.Tension = 0.0f;
		SaveGame->Fishing.CurrentBiteTime = 0.0f;
		SaveGame->Fishing.bOfferCompletedCatchesToStarwell = FishingComponent->bOfferCompletedCatchesToStarwell;
		SaveGame->Fishing.Progress = FishingComponent->FishingProgress;
	}

	if (EconomyComponent != nullptr)
	{
		SaveGame->Economy.Stardust = EconomyComponent->Stardust;
		SaveGame->Economy.MoonPearls = EconomyComponent->MoonPearls;
		SaveGame->Economy.EchoScales = EconomyComponent->EchoScales;
		SaveGame->Economy.AppliedUpgradeCounts = EconomyComponent->AppliedUpgradeCounts;
	}

	if (DeckComponent != nullptr)
	{
		CaptureCardArray(DeckComponent->DrawPile, SaveGame->Deck.DrawPile);
		CaptureCardArray(DeckComponent->Hand, SaveGame->Deck.Hand);
		CaptureCardArray(DeckComponent->DiscardPile, SaveGame->Deck.DiscardPile);
		SaveGame->Deck.ShuffleSeed = DeckComponent->ShuffleSeed;
		SaveGame->Deck.ShuffleCount = DeckComponent->ShuffleCount;
	}

	if (Starwell != nullptr)
	{
		SaveGame->Starwell.EchoScalesPerBaseStardustValue = Starwell->EchoScalesPerBaseStardustValue;
		SaveGame->Starwell.MinimumEchoScalesPerFish = Starwell->MinimumEchoScalesPerFish;
		SaveGame->Starwell.OfferingProgress = Starwell->OfferingProgress;
		SaveGame->Starwell.TotalFishAccepted = Starwell->TotalFishAccepted;
		SaveGame->Starwell.TotalEchoScalesGenerated = Starwell->TotalEchoScalesGenerated;
		SaveGame->Starwell.ReachedStoryUnlockIds = Starwell->ReachedStoryUnlockIds;
	}

	return SaveGame;
}

bool UNyxSaveGameLibrary::ApplyNyxSaveGame(UNyxSaveGame* SaveGame, UFishingComponent* FishingComponent, UEconomyComponent* EconomyComponent, UDeckComponent* DeckComponent, AStarwell* Starwell)
{
	if (SaveGame == nullptr)
	{
		return false;
	}

	if (FishingComponent != nullptr)
	{
		TArray<UFishDataAsset*> RestoredAvailableFish;
		RestoreFishArray(SaveGame->Fishing.AvailableFish, RestoredAvailableFish);

		FishingComponent->RestoreSavedState(
			RestoredAvailableFish,
			SaveGame->Fishing.RandomSeed,
			SaveGame->Fishing.CastIndex,
			SaveGame->Fishing.bOfferCompletedCatchesToStarwell,
			SaveGame->Fishing.Progress);
	}

	if (EconomyComponent != nullptr)
	{
		EconomyComponent->RestoreSavedState(
			SaveGame->Economy.Stardust,
			SaveGame->Economy.MoonPearls,
			SaveGame->Economy.EchoScales,
			SaveGame->Economy.AppliedUpgradeCounts);
	}

	if (DeckComponent != nullptr)
	{
		TArray<UCardDataAsset*> RestoredDrawPile;
		TArray<UCardDataAsset*> RestoredHand;
		TArray<UCardDataAsset*> RestoredDiscardPile;
		RestoreCardArray(SaveGame->Deck.DrawPile, RestoredDrawPile);
		RestoreCardArray(SaveGame->Deck.Hand, RestoredHand);
		RestoreCardArray(SaveGame->Deck.DiscardPile, RestoredDiscardPile);

		DeckComponent->RestoreSavedState(
			RestoredDrawPile,
			RestoredHand,
			RestoredDiscardPile,
			SaveGame->Deck.ShuffleSeed,
			SaveGame->Deck.ShuffleCount);
	}

	if (Starwell != nullptr)
	{
		Starwell->RestoreSavedProgress(
			SaveGame->Starwell.EchoScalesPerBaseStardustValue,
			SaveGame->Starwell.MinimumEchoScalesPerFish,
			SaveGame->Starwell.OfferingProgress,
			SaveGame->Starwell.TotalFishAccepted,
			SaveGame->Starwell.TotalEchoScalesGenerated,
			SaveGame->Starwell.ReachedStoryUnlockIds);
	}

	return true;
}

bool UNyxSaveGameLibrary::SaveNyxGameToSlot(const FString& SlotName, int32 UserIndex, UFishingComponent* FishingComponent, UEconomyComponent* EconomyComponent, UDeckComponent* DeckComponent, AStarwell* Starwell)
{
	if (SlotName.IsEmpty())
	{
		return false;
	}

	UNyxSaveGame* SaveGame = CaptureNyxSaveGame(FishingComponent, EconomyComponent, DeckComponent, Starwell);
	if (SaveGame == nullptr)
	{
		return false;
	}

	return UGameplayStatics::SaveGameToSlot(SaveGame, SlotName, UserIndex);
}

UNyxSaveGame* UNyxSaveGameLibrary::LoadNyxGameFromSlot(const FString& SlotName, int32 UserIndex)
{
	if (SlotName.IsEmpty() || !UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
	{
		return nullptr;
	}

	return Cast<UNyxSaveGame>(UGameplayStatics::LoadGameFromSlot(SlotName, UserIndex));
}

bool UNyxSaveGameLibrary::LoadNyxGameFromSlotAndApply(const FString& SlotName, int32 UserIndex, UFishingComponent* FishingComponent, UEconomyComponent* EconomyComponent, UDeckComponent* DeckComponent, AStarwell* Starwell)
{
	UNyxSaveGame* SaveGame = LoadNyxGameFromSlot(SlotName, UserIndex);
	return ApplyNyxSaveGame(SaveGame, FishingComponent, EconomyComponent, DeckComponent, Starwell);
}
