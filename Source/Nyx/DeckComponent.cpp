#include "DeckComponent.h"

#include "CardDataAsset.h"

UDeckComponent::UDeckComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	ShuffleSeed = 2026;
	ShuffleCount = 0;
}

UCardDataAsset* UDeckComponent::DrawCard()
{
	if (DrawPile.IsEmpty())
	{
		ShuffleDiscardIntoDeck();
	}

	if (DrawPile.IsEmpty())
	{
		return nullptr;
	}

	UCardDataAsset* DrawnCard = DrawPile[0].Get();
	DrawPile.RemoveAt(0);

	if (DrawnCard != nullptr)
	{
		Hand.Add(DrawnCard);
	}

	OnCardDrawn.Broadcast(this, DrawnCard);
	BroadcastDeckChanged();
	return DrawnCard;
}

bool UDeckComponent::PlayCard(UCardDataAsset* Card)
{
	if (Card == nullptr)
	{
		return false;
	}

	int32 HandIndex = INDEX_NONE;
	for (int32 Index = 0; Index < Hand.Num(); ++Index)
	{
		if (Hand[Index].Get() == Card)
		{
			HandIndex = Index;
			break;
		}
	}

	if (HandIndex == INDEX_NONE)
	{
		return false;
	}

	Hand.RemoveAt(HandIndex);
	DiscardPile.Add(Card);

	OnCardPlayed.Broadcast(this, Card);
	BroadcastDeckChanged();
	return true;
}

void UDeckComponent::ShuffleDiscardIntoDeck()
{
	if (DiscardPile.IsEmpty())
	{
		return;
	}

	DrawPile.Append(DiscardPile);
	DiscardPile.Reset();

	FRandomStream RandomStream = MakeShuffleRandomStream();
	ShuffleCards(DrawPile, RandomStream);
	++ShuffleCount;

	OnDiscardShuffledIntoDeck.Broadcast(this);
	BroadcastDeckChanged();
}

void UDeckComponent::RestoreSavedState(const TArray<UCardDataAsset*>& RestoredDrawPile, const TArray<UCardDataAsset*>& RestoredHand, const TArray<UCardDataAsset*>& RestoredDiscardPile, int32 RestoredShuffleSeed, int32 RestoredShuffleCount)
{
	DrawPile.Reset();
	DrawPile.Reserve(RestoredDrawPile.Num());
	for (UCardDataAsset* Card : RestoredDrawPile)
	{
		if (Card != nullptr)
		{
			DrawPile.Add(Card);
		}
	}

	Hand.Reset();
	Hand.Reserve(RestoredHand.Num());
	for (UCardDataAsset* Card : RestoredHand)
	{
		if (Card != nullptr)
		{
			Hand.Add(Card);
		}
	}

	DiscardPile.Reset();
	DiscardPile.Reserve(RestoredDiscardPile.Num());
	for (UCardDataAsset* Card : RestoredDiscardPile)
	{
		if (Card != nullptr)
		{
			DiscardPile.Add(Card);
		}
	}

	ShuffleSeed = RestoredShuffleSeed;
	ShuffleCount = FMath::Max(0, RestoredShuffleCount);

	BroadcastDeckChanged();
	OnDeckSaveApplied.Broadcast(this);
	OnDeckSaveAppliedNative.Broadcast(this);
}

int32 UDeckComponent::GetDrawPileCount() const
{
	return DrawPile.Num();
}

int32 UDeckComponent::GetHandCount() const
{
	return Hand.Num();
}

int32 UDeckComponent::GetDiscardPileCount() const
{
	return DiscardPile.Num();
}

FRandomStream UDeckComponent::MakeShuffleRandomStream() const
{
	return FRandomStream(ShuffleSeed + ShuffleCount);
}

void UDeckComponent::ShuffleCards(TArray<TObjectPtr<UCardDataAsset>>& Cards, FRandomStream& RandomStream) const
{
	for (int32 Index = Cards.Num() - 1; Index > 0; --Index)
	{
		const int32 SwapIndex = RandomStream.RandRange(0, Index);
		Cards.Swap(Index, SwapIndex);
	}
}

void UDeckComponent::BroadcastDeckChanged()
{
	OnDeckChanged.Broadcast(this);
}
