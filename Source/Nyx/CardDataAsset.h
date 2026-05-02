#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "CardDataAsset.generated.h"

UENUM(BlueprintType)
enum class ECardType : uint8
{
	Lure UMETA(DisplayName="Lure"),
	Tide UMETA(DisplayName="Tide"),
	Ritual UMETA(DisplayName="Ritual"),
	Tool UMETA(DisplayName="Tool"),
	Companion UMETA(DisplayName="Companion"),
	Oddity UMETA(DisplayName="Oddity")
};

UENUM(BlueprintType)
enum class ECardPrototypeEffect : uint8
{
	None UMETA(DisplayName="None"),
	ModifyBiteTime UMETA(DisplayName="Modify Bite Time"),
	ModifyReelDifficulty UMETA(DisplayName="Modify Reel Difficulty"),
	ModifyTension UMETA(DisplayName="Modify Tension"),
	ModifyStardustValue UMETA(DisplayName="Modify Stardust Value"),
	FilterFishByTag UMETA(DisplayName="Filter Fish By Tag"),
	ForceFishRarity UMETA(DisplayName="Force Fish Rarity")
};

UCLASS(BlueprintType)
class NYX_API UCardDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Player-facing card name shown in hand, deck, and reward screens.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Card")
	FText CardName;

	// Resource cost paid before the card effect is applied.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Card", meta=(ClampMin="0", UIMin="0"))
	int32 Cost = 0;

	// Rules text or flavor text shown when inspecting the card.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Card", meta=(MultiLine="true"))
	FText Description;

	// Broad card family for deckbuilding rules, UI grouping, and future synergies.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Card")
	ECardType CardType = ECardType::Lure;

	// Tags this card reads, adds, blocks, or modifies when interacting with fish and fishing rules.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Card|Effects")
	FGameplayTagContainer AffectedGameplayTags;

	// Lightweight prototype effect hook until card behavior becomes full data-driven effect objects.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Card|Effects")
	ECardPrototypeEffect PrototypeEffect = ECardPrototypeEffect::None;
};
