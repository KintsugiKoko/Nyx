#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DeckComponent.generated.h"

class UCardDataAsset;
class UDeckComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FDeckCardEventSignature, UDeckComponent*, DeckComponent, UCardDataAsset*, Card);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FDeckChangedSignature, UDeckComponent*, DeckComponent);

UCLASS(ClassGroup=(Cards), Blueprintable, meta=(BlueprintSpawnableComponent))
class NYX_API UDeckComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UDeckComponent();

	// Cards available to draw. Set this in editor or rebuild it from save data.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Deck")
	TArray<TObjectPtr<UCardDataAsset>> DrawPile;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Deck")
	TArray<TObjectPtr<UCardDataAsset>> Hand;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Deck")
	TArray<TObjectPtr<UCardDataAsset>> DiscardPile;

	// Used with ShuffleCount so discard shuffles can be repeated for balancing and save/load.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, SaveGame, Category="Deck|Determinism")
	int32 ShuffleSeed;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, SaveGame, Category="Deck|Determinism")
	int32 ShuffleCount;

	UPROPERTY(BlueprintAssignable, Category="Deck|Events")
	FDeckCardEventSignature OnCardDrawn;

	UPROPERTY(BlueprintAssignable, Category="Deck|Events")
	FDeckCardEventSignature OnCardPlayed;

	UPROPERTY(BlueprintAssignable, Category="Deck|Events")
	FDeckChangedSignature OnDiscardShuffledIntoDeck;

	UPROPERTY(BlueprintAssignable, Category="Deck|Events")
	FDeckChangedSignature OnDeckChanged;

	UPROPERTY(BlueprintAssignable, Category="Deck|Events")
	FDeckChangedSignature OnDeckSaveApplied;

	UFUNCTION(BlueprintCallable, Category="Deck")
	UCardDataAsset* DrawCard();

	UFUNCTION(BlueprintCallable, Category="Deck")
	bool PlayCard(UCardDataAsset* Card);

	UFUNCTION(BlueprintCallable, Category="Deck")
	void ShuffleDiscardIntoDeck();

	UFUNCTION(BlueprintCallable, Category="Deck|Save")
	void RestoreSavedState(const TArray<UCardDataAsset*>& RestoredDrawPile, const TArray<UCardDataAsset*>& RestoredHand, const TArray<UCardDataAsset*>& RestoredDiscardPile, int32 RestoredShuffleSeed, int32 RestoredShuffleCount);

	UFUNCTION(BlueprintPure, Category="Deck")
	int32 GetDrawPileCount() const;

	UFUNCTION(BlueprintPure, Category="Deck")
	int32 GetHandCount() const;

	UFUNCTION(BlueprintPure, Category="Deck")
	int32 GetDiscardPileCount() const;

private:
	FRandomStream MakeShuffleRandomStream() const;
	void ShuffleCards(TArray<TObjectPtr<UCardDataAsset>>& Cards, FRandomStream& RandomStream) const;
	void BroadcastDeckChanged();
};
