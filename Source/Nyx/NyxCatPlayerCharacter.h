#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "NyxCatPlayerCharacter.generated.h"

class UDeckComponent;
class UEconomyComponent;
class UFishingComponent;
class USkeletalMeshComponent;
class ANyxCatPlayerCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNyxCatAnimationCueSignature, ANyxCatPlayerCharacter*, CatCharacter);

UCLASS(Blueprintable)
class NYX_API ANyxCatPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ANyxCatPlayerCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nyx|Components")
	TObjectPtr<UFishingComponent> FishingComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nyx|Components")
	TObjectPtr<UEconomyComponent> EconomyComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Nyx|Components")
	TObjectPtr<UDeckComponent> DeckComponent;

	// Socket on the cat skeletal mesh where a fishing rod prop can attach.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nyx|Animation|Sockets")
	FName FishingRodSocketName;

	// Socket used for held fish props or offering anticipation poses.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nyx|Animation|Sockets")
	FName CatchHoldSocketName;

	// Socket used when handing a fish toward the Starwell.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nyx|Animation|Sockets")
	FName OfferingSocketName;

	UPROPERTY(BlueprintAssignable, Category="Nyx|Animation|Events")
	FNyxCatAnimationCueSignature OnFishingCastAnimationCue;

	UPROPERTY(BlueprintAssignable, Category="Nyx|Animation|Events")
	FNyxCatAnimationCueSignature OnFishingReelAnimationCue;

	UPROPERTY(BlueprintAssignable, Category="Nyx|Animation|Events")
	FNyxCatAnimationCueSignature OnFishingCatchAnimationCue;

	UFUNCTION(BlueprintPure, Category="Nyx|Animation")
	USkeletalMeshComponent* GetCatMesh() const;

	UFUNCTION(BlueprintCallable, Category="Nyx|Animation")
	void HandleFishingCastAnimationCue();

	UFUNCTION(BlueprintCallable, Category="Nyx|Animation")
	void HandleFishingReelAnimationCue();

	UFUNCTION(BlueprintCallable, Category="Nyx|Animation")
	void HandleFishingCatchAnimationCue();
};
