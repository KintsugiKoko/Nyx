#include "NyxCatPlayerCharacter.h"

#include "Components/SkeletalMeshComponent.h"
#include "DeckComponent.h"
#include "EconomyComponent.h"
#include "FishingComponent.h"
#include "KoiSkillTreeComponent.h"
#include "NyxFTUEComponent.h"

ANyxCatPlayerCharacter::ANyxCatPlayerCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	FishingComponent = CreateDefaultSubobject<UFishingComponent>(TEXT("FishingComponent"));
	EconomyComponent = CreateDefaultSubobject<UEconomyComponent>(TEXT("EconomyComponent"));
	DeckComponent = CreateDefaultSubobject<UDeckComponent>(TEXT("DeckComponent"));
	SkillTreeComponent = CreateDefaultSubobject<UKoiSkillTreeComponent>(TEXT("SkillTreeComponent"));
	FTUEComponent = CreateDefaultSubobject<UNyxFTUEComponent>(TEXT("FTUEComponent"));

	FishingRodSocketName = TEXT("FishingRodSocket");
	CatchHoldSocketName = TEXT("CatchHoldSocket");
	OfferingSocketName = TEXT("OfferingSocket");
}

void ANyxCatPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();
	RefreshFTUEBindings();
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

void ANyxCatPlayerCharacter::RefreshFTUEBindings()
{
	if (SkillTreeComponent != nullptr && FishingComponent != nullptr)
	{
		SkillTreeComponent->ApplySkillModifiers(FishingComponent.Get(), FishingComponent->StarwellTarget.Get());
	}

	if (FTUEComponent != nullptr)
	{
		FTUEComponent->BindToCoreLoop(FishingComponent.Get(), EconomyComponent.Get(), DeckComponent.Get(), FishingComponent != nullptr ? FishingComponent->StarwellTarget.Get() : nullptr);
	}
}
