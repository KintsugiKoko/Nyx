#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "FishDataAsset.generated.h"

class UMaterialInterface;
class UStaticMesh;
class UTexture2D;

UENUM(BlueprintType)
enum class EFishRarity : uint8
{
	Common UMETA(DisplayName="Common"),
	Uncommon UMETA(DisplayName="Uncommon"),
	Rare UMETA(DisplayName="Rare"),
	Epic UMETA(DisplayName="Epic"),
	Legendary UMETA(DisplayName="Legendary")
};

UCLASS(BlueprintType)
class NYX_API UFishDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Stable save/load identity for discovery, catch counts, and collection progress. Keep unchanged after content ships.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fish|Save")
	FName FishId;

	// Player-facing fish name shown in UI, inventory cards, and catch messages.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fish")
	FText DisplayName;

	// Flavor or gameplay description shown when inspecting the fish.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fish", meta=(MultiLine="true"))
	FText Description;

	// Base currency value before rarity, card, or progression modifiers are applied.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fish", meta=(ClampMin="0", UIMin="0"))
	int32 BaseStardustValue = 1;

	// Rarity tier used by loot tables, presentation, and value tuning.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fish")
	EFishRarity Rarity = EFishRarity::Common;

	// Bite timing in seconds: X is minimum time, Y is maximum time.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fish", meta=(ClampMin="0.0", UIMin="0.0"))
	FVector2D BiteTimeRange = FVector2D(1.0f, 3.0f);

	// Higher values can make reel checks, stamina drain, or tension windows harder.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fish", meta=(ClampMin="0.0", UIMin="0.0"))
	float ReelDifficulty = 1.0f;

	// Gameplay tags that cards can query, such as Fish.River, Fish.Night, or Fish.Magic.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fish|Cards")
	FGameplayTagContainer CardInteractionTags;

	// Optional presentation mesh for catch previews, world fish props, or UI dioramas. Gameplay should not depend on it.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fish|Presentation")
	TSoftObjectPtr<UStaticMesh> PresentationMesh;

	// Optional presentation material override for rarity/readability tests.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fish|Presentation")
	TSoftObjectPtr<UMaterialInterface> PresentationMaterial;

	// Optional flat icon for UI cards or catch logs.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fish|Presentation")
	TSoftObjectPtr<UTexture2D> IconTexture;

	UFUNCTION(BlueprintPure, Category="Fish|Save")
	FName GetStableFishId() const;
};
