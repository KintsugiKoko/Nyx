#pragma once

#include "CoreMinimal.h"
#include "CardDataAsset.h"
#include "FishDataAsset.h"
#include "FishingComponent.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NyxSaveGame.generated.h"

class AStarwell;
class UDeckComponent;
class UEconomyComponent;

USTRUCT(BlueprintType)
struct FNyxFishingSaveData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Fishing")
	TArray<TSoftObjectPtr<UFishDataAsset>> AvailableFish;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Fishing")
	TSoftObjectPtr<UFishDataAsset> SelectedFish;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Fishing")
	EFishingState FishingState = EFishingState::Idle;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Fishing")
	int32 RandomSeed = 1337;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Fishing")
	int32 CastIndex = 0;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Fishing")
	float Tension = 0.0f;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Fishing")
	float CurrentBiteTime = 0.0f;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Fishing")
	bool bOfferCompletedCatchesToStarwell = true;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Fishing")
	FNyxFishingProgressData Progress;
};

USTRUCT(BlueprintType)
struct FNyxEconomySaveData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Economy")
	int32 Stardust = 0;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Economy")
	int32 MoonPearls = 0;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Economy")
	int32 EchoScales = 0;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Economy")
	TMap<FName, int32> AppliedUpgradeCounts;
};

USTRUCT(BlueprintType)
struct FNyxDeckSaveData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Deck")
	TArray<TSoftObjectPtr<UCardDataAsset>> DrawPile;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Deck")
	TArray<TSoftObjectPtr<UCardDataAsset>> Hand;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Deck")
	TArray<TSoftObjectPtr<UCardDataAsset>> DiscardPile;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Deck")
	int32 ShuffleSeed = 2026;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Deck")
	int32 ShuffleCount = 0;
};

USTRUCT(BlueprintType)
struct FNyxStarwellSaveData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Starwell")
	int32 EchoScalesPerBaseStardustValue = 1;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Starwell")
	int32 MinimumEchoScalesPerFish = 1;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Starwell")
	int32 OfferingProgress = 0;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Starwell")
	int32 TotalFishAccepted = 0;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Starwell")
	int32 TotalEchoScalesGenerated = 0;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save|Starwell")
	TArray<FName> ReachedStoryUnlockIds;
};

UCLASS(BlueprintType)
class NYX_API UNyxSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save")
	FNyxFishingSaveData Fishing;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save")
	FNyxEconomySaveData Economy;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save")
	FNyxDeckSaveData Deck;

	UPROPERTY(BlueprintReadWrite, SaveGame, Category="Nyx|Save")
	FNyxStarwellSaveData Starwell;
};

UCLASS()
class NYX_API UNyxSaveGameLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Nyx|Save")
	static UNyxSaveGame* CreateNyxSaveGame();

	UFUNCTION(BlueprintCallable, Category="Nyx|Save")
	static UNyxSaveGame* CaptureNyxSaveGame(UFishingComponent* FishingComponent, UEconomyComponent* EconomyComponent, UDeckComponent* DeckComponent, AStarwell* Starwell);

	UFUNCTION(BlueprintCallable, Category="Nyx|Save")
	static bool ApplyNyxSaveGame(UNyxSaveGame* SaveGame, UFishingComponent* FishingComponent, UEconomyComponent* EconomyComponent, UDeckComponent* DeckComponent, AStarwell* Starwell);

	UFUNCTION(BlueprintCallable, Category="Nyx|Save")
	static bool SaveNyxGameToSlot(const FString& SlotName, int32 UserIndex, UFishingComponent* FishingComponent, UEconomyComponent* EconomyComponent, UDeckComponent* DeckComponent, AStarwell* Starwell);

	UFUNCTION(BlueprintCallable, Category="Nyx|Save")
	static UNyxSaveGame* LoadNyxGameFromSlot(const FString& SlotName, int32 UserIndex);

	UFUNCTION(BlueprintCallable, Category="Nyx|Save")
	static bool LoadNyxGameFromSlotAndApply(const FString& SlotName, int32 UserIndex, UFishingComponent* FishingComponent, UEconomyComponent* EconomyComponent, UDeckComponent* DeckComponent, AStarwell* Starwell);
};
