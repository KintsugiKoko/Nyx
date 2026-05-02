#include "NyxFTUEComponent.h"

#include "CardDataAsset.h"
#include "DeckComponent.h"
#include "EconomyComponent.h"
#include "FishDataAsset.h"
#include "FishingComponent.h"
#include "Starwell.h"

UNyxFTUEComponent::UNyxFTUEComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	bFTUEEnabled = true;
	bAutoAdvanceFromCoreLoopEvents = true;
	CurrentStep = ENyxFTUEStep::CastLine;
	bFTUECompleted = false;

	BuildDefaultStepDefinitions();
}

void UNyxFTUEComponent::StartFTUE()
{
	if (!bFTUEEnabled)
	{
		return;
	}

	bFTUECompleted = false;
	AdvanceToStep(ENyxFTUEStep::CastLine);
}

void UNyxFTUEComponent::ResetFTUE()
{
	bFTUEEnabled = true;
	bFTUECompleted = false;
	CurrentStep = ENyxFTUEStep::CastLine;
	OnFTUEStepChanged.Broadcast(this, CurrentStep);
}

void UNyxFTUEComponent::CompleteFTUE()
{
	if (bFTUECompleted)
	{
		return;
	}

	CurrentStep = ENyxFTUEStep::Complete;
	bFTUECompleted = true;
	OnFTUEStepChanged.Broadcast(this, CurrentStep);
	OnFTUECompleted.Broadcast(this);
}

void UNyxFTUEComponent::AdvanceToStep(ENyxFTUEStep NewStep)
{
	if (!bFTUEEnabled || bFTUECompleted)
	{
		return;
	}

	if (NewStep == ENyxFTUEStep::Complete)
	{
		CompleteFTUE();
		return;
	}

	if (CurrentStep == NewStep)
	{
		OnFTUEStepChanged.Broadcast(this, CurrentStep);
		return;
	}

	CurrentStep = NewStep;
	OnFTUEStepChanged.Broadcast(this, CurrentStep);
}

void UNyxFTUEComponent::AdvanceToNextStep()
{
	AdvanceToStep(GetNextStep(CurrentStep));
}

void UNyxFTUEComponent::SetFTUEEnabled(bool bNewEnabled)
{
	bFTUEEnabled = bNewEnabled;
	if (!bFTUEEnabled)
	{
		CompleteFTUE();
	}
}

void UNyxFTUEComponent::BindToCoreLoop(UFishingComponent* FishingComponent, UEconomyComponent* EconomyComponent, UDeckComponent* DeckComponent, AStarwell* Starwell)
{
	UnbindCoreLoop();

	BoundFishingComponent = FishingComponent;
	BoundEconomyComponent = EconomyComponent;
	BoundDeckComponent = DeckComponent;
	BoundStarwell = Starwell;

	if (BoundFishingComponent != nullptr)
	{
		BoundFishingComponent->OnCastStarted.AddUniqueDynamic(this, &UNyxFTUEComponent::HandleCastStarted);
		BoundFishingComponent->OnReelStarted.AddUniqueDynamic(this, &UNyxFTUEComponent::HandleReelStarted);
		BoundFishingComponent->OnCatchCompleted.AddUniqueDynamic(this, &UNyxFTUEComponent::HandleCatchCompleted);
		BoundFishingComponent->OnCatchOfferedToStarwell.AddUniqueDynamic(this, &UNyxFTUEComponent::HandleCatchOfferedToStarwell);
	}

	if (BoundDeckComponent != nullptr)
	{
		BoundDeckComponent->OnCardDrawn.AddUniqueDynamic(this, &UNyxFTUEComponent::HandleCardDrawn);
		BoundDeckComponent->OnCardPlayed.AddUniqueDynamic(this, &UNyxFTUEComponent::HandleCardPlayed);
	}

	if (BoundStarwell != nullptr)
	{
		BoundStarwell->OnFishAccepted.AddUniqueDynamic(this, &UNyxFTUEComponent::HandleFishAccepted);
	}
}

FNyxFTUEStepDefinition UNyxFTUEComponent::GetCurrentStepDefinition() const
{
	if (const FNyxFTUEStepDefinition* Definition = FindStepDefinition(CurrentStep))
	{
		return *Definition;
	}

	FNyxFTUEStepDefinition MissingDefinition;
	MissingDefinition.Step = CurrentStep;
	MissingDefinition.Title = FText::FromString(TEXT("Keep going"));
	MissingDefinition.Body = FText::FromString(TEXT("Follow the next available interaction."));
	MissingDefinition.ActionLabel = FText::FromString(TEXT("Continue"));
	return MissingDefinition;
}

bool UNyxFTUEComponent::IsFTUEActive() const
{
	return bFTUEEnabled && !bFTUECompleted;
}

void UNyxFTUEComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindCoreLoop();
	Super::EndPlay(EndPlayReason);
}

void UNyxFTUEComponent::BuildDefaultStepDefinitions()
{
	StepDefinitions.Reset();

	auto AddStep = [this](ENyxFTUEStep Step, const TCHAR* Title, const TCHAR* Body, const TCHAR* ActionLabel)
	{
		FNyxFTUEStepDefinition& Definition = StepDefinitions.AddDefaulted_GetRef();
		Definition.Step = Step;
		Definition.Title = FText::FromString(Title);
		Definition.Body = FText::FromString(Body);
		Definition.ActionLabel = FText::FromString(ActionLabel);
	};

	AddStep(ENyxFTUEStep::CastLine, TEXT("Cast your line"), TEXT("Find a calm spot and cast. Nyx will wait for a bite."), TEXT("Cast"));
	AddStep(ENyxFTUEStep::StartReel, TEXT("Wait for the bite"), TEXT("When a fish bites, start reeling. Keep it simple for now."), TEXT("Reel"));
	AddStep(ENyxFTUEStep::CompleteCatch, TEXT("Bring it in"), TEXT("Finish the reel to land the fish."), TEXT("Catch"));
	AddStep(ENyxFTUEStep::OfferCatch, TEXT("Offer the catch"), TEXT("Send the fish to the Starwell to turn it into Echo Scales."), TEXT("Offer"));
	AddStep(ENyxFTUEStep::DrawCard, TEXT("Draw a card"), TEXT("Cards shape future fishing runs. Draw one to see your options."), TEXT("Draw"));
	AddStep(ENyxFTUEStep::PlayCard, TEXT("Play a card"), TEXT("Play a card from hand to complete the first loop."), TEXT("Play"));
	AddStep(ENyxFTUEStep::Complete, TEXT("You are set"), TEXT("Fish, offer, earn, and experiment with cards. The rest is yours to explore."), TEXT("Done"));
}

void UNyxFTUEComponent::UnbindCoreLoop()
{
	if (BoundFishingComponent != nullptr)
	{
		BoundFishingComponent->OnCastStarted.RemoveDynamic(this, &UNyxFTUEComponent::HandleCastStarted);
		BoundFishingComponent->OnReelStarted.RemoveDynamic(this, &UNyxFTUEComponent::HandleReelStarted);
		BoundFishingComponent->OnCatchCompleted.RemoveDynamic(this, &UNyxFTUEComponent::HandleCatchCompleted);
		BoundFishingComponent->OnCatchOfferedToStarwell.RemoveDynamic(this, &UNyxFTUEComponent::HandleCatchOfferedToStarwell);
	}

	if (BoundDeckComponent != nullptr)
	{
		BoundDeckComponent->OnCardDrawn.RemoveDynamic(this, &UNyxFTUEComponent::HandleCardDrawn);
		BoundDeckComponent->OnCardPlayed.RemoveDynamic(this, &UNyxFTUEComponent::HandleCardPlayed);
	}

	if (BoundStarwell != nullptr)
	{
		BoundStarwell->OnFishAccepted.RemoveDynamic(this, &UNyxFTUEComponent::HandleFishAccepted);
	}

	BoundFishingComponent = nullptr;
	BoundEconomyComponent = nullptr;
	BoundDeckComponent = nullptr;
	BoundStarwell = nullptr;
}

const FNyxFTUEStepDefinition* UNyxFTUEComponent::FindStepDefinition(ENyxFTUEStep Step) const
{
	return StepDefinitions.FindByPredicate([Step](const FNyxFTUEStepDefinition& Definition)
	{
		return Definition.Step == Step;
	});
}

ENyxFTUEStep UNyxFTUEComponent::GetNextStep(ENyxFTUEStep Step)
{
	switch (Step)
	{
	case ENyxFTUEStep::CastLine:
		return ENyxFTUEStep::StartReel;
	case ENyxFTUEStep::StartReel:
		return ENyxFTUEStep::CompleteCatch;
	case ENyxFTUEStep::CompleteCatch:
		return ENyxFTUEStep::OfferCatch;
	case ENyxFTUEStep::OfferCatch:
		return ENyxFTUEStep::DrawCard;
	case ENyxFTUEStep::DrawCard:
		return ENyxFTUEStep::PlayCard;
	case ENyxFTUEStep::PlayCard:
	default:
		return ENyxFTUEStep::Complete;
	}
}

void UNyxFTUEComponent::AdvanceIfCurrentStepIs(ENyxFTUEStep ExpectedStep, ENyxFTUEStep NewStep)
{
	if (bAutoAdvanceFromCoreLoopEvents && CurrentStep == ExpectedStep)
	{
		AdvanceToStep(NewStep);
	}
}

void UNyxFTUEComponent::HandleCastStarted(UFishingComponent* FishingComponent)
{
	AdvanceIfCurrentStepIs(ENyxFTUEStep::CastLine, ENyxFTUEStep::StartReel);
}

void UNyxFTUEComponent::HandleReelStarted(UFishingComponent* FishingComponent, UFishDataAsset* Fish)
{
	AdvanceIfCurrentStepIs(ENyxFTUEStep::StartReel, ENyxFTUEStep::CompleteCatch);
}

void UNyxFTUEComponent::HandleCatchCompleted(UFishingComponent* FishingComponent, UFishDataAsset* Fish)
{
	AdvanceIfCurrentStepIs(ENyxFTUEStep::CompleteCatch, ENyxFTUEStep::OfferCatch);
}

void UNyxFTUEComponent::HandleCatchOfferedToStarwell(UFishingComponent* FishingComponent, UFishDataAsset* Fish, AStarwell* Starwell, UEconomyComponent* EconomyComponent, int32 EchoScalesGranted)
{
	AdvanceIfCurrentStepIs(ENyxFTUEStep::OfferCatch, ENyxFTUEStep::DrawCard);
}

void UNyxFTUEComponent::HandleFishAccepted(AStarwell* Starwell, UFishDataAsset* Fish, int32 EchoScalesGranted, int32 NewOfferingProgress, UEconomyComponent* EconomyComponent)
{
	AdvanceIfCurrentStepIs(ENyxFTUEStep::OfferCatch, ENyxFTUEStep::DrawCard);
}

void UNyxFTUEComponent::HandleCardDrawn(UDeckComponent* DeckComponent, UCardDataAsset* Card)
{
	AdvanceIfCurrentStepIs(ENyxFTUEStep::DrawCard, ENyxFTUEStep::PlayCard);
}

void UNyxFTUEComponent::HandleCardPlayed(UDeckComponent* DeckComponent, UCardDataAsset* Card)
{
	AdvanceIfCurrentStepIs(ENyxFTUEStep::PlayCard, ENyxFTUEStep::Complete);
}
