#include "NyxCatPlayerCharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "DeckComponent.h"
#include "EconomyComponent.h"
#include "FishingComponent.h"

ANyxCatPlayerCharacter::ANyxCatPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	FishingComponent = CreateDefaultSubobject<UFishingComponent>(TEXT("FishingComponent"));
	EconomyComponent = CreateDefaultSubobject<UEconomyComponent>(TEXT("EconomyComponent"));
	DeckComponent = CreateDefaultSubobject<UDeckComponent>(TEXT("DeckComponent"));

	FishingRodSocketName = TEXT("FishingRodSocket");
	CatchHoldSocketName = TEXT("CatchHoldSocket");
	OfferingSocketName = TEXT("OfferingSocket");
}

USkeletalMeshComponent* ANyxCatPlayerCharacter::GetCatMesh() const
{
	return GetMesh();
}

void ANyxCatPlayerCharacter::HandleFishingCastAnimationCue()
{
	OnFishingCastAnimationCue.Broadcast(this);
}

void ANyxCatPlayerCharacter::HandleFishingReelAnimationCue()
{
	OnFishingReelAnimationCue.Broadcast(this);
}

void ANyxCatPlayerCharacter::HandleFishingCatchAnimationCue()
{
	OnFishingCatchAnimationCue.Broadcast(this);
}
