#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "NyxFTUEComponent.generated.h"

class AStarwell;
class UCardDataAsset;
class UDeckComponent;
class UEconomyComponent;
class UFishDataAsset;
class UFishingComponent;
class UNyxFTUEComponent;

UENUM(BlueprintType)
enum class ENyxFTUEStep : uint8
{
	CastLine UMETA(DisplayName="Cast Line"),
	StartReel UMETA(DisplayName="Start Reel"),
	CompleteCatch UMETA(DisplayName="Complete Catch"),
	OfferCatch UMETA(DisplayName="Offer Catch"),
	DrawCard UMETA(DisplayName="Draw Card"),
	PlayCard UMETA(DisplayName="Play Card"),
	Complete UMETA(DisplayName="Complete")
};

USTRUCT(BlueprintType)
struct FNyxFTUEStepDefinition
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nyx|FTUE")
	ENyxFTUEStep Step = ENyxFTUEStep::CastLine;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nyx|FTUE")
	FText Title;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nyx|FTUE", meta=(MultiLine="true"))
	FText Body;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nyx|FTUE")
	FText ActionLabel;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNyxFTUEStepChangedSignature, UNyxFTUEComponent*, FTUEComponent, ENyxFTUEStep, NewStep);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNyxFTUEEventSignature, UNyxFTUEComponent*, FTUEComponent);

UCLASS(ClassGroup=(Nyx), Blueprintable, meta=(BlueprintSpawnableComponent))
class NYX_API UNyxFTUEComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UNyxFTUEComponent();

	// Keep this true for the first session; turn it off in Blueprint/save data once the player has opted out.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nyx|FTUE")
	bool bFTUEEnabled;

	// When true, gameplay events advance the current prompt. The FTUE never blocks input or triggers actions itself.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nyx|FTUE")
	bool bAutoAdvanceFromCoreLoopEvents;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Nyx|FTUE")
	ENyxFTUEStep CurrentStep;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, Category="Nyx|FTUE")
	bool bFTUECompleted;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Nyx|FTUE")
	TArray<FNyxFTUEStepDefinition> StepDefinitions;

	UPROPERTY(BlueprintAssignable, Category="Nyx|FTUE|Events")
	FNyxFTUEStepChangedSignature OnFTUEStepChanged;

	UPROPERTY(BlueprintAssignable, Category="Nyx|FTUE|Events")
	FNyxFTUEEventSignature OnFTUECompleted;

	UFUNCTION(BlueprintCallable, Category="Nyx|FTUE")
	void StartFTUE();

	UFUNCTION(BlueprintCallable, Category="Nyx|FTUE")
	void ResetFTUE();

	UFUNCTION(BlueprintCallable, Category="Nyx|FTUE")
	void CompleteFTUE();

	UFUNCTION(BlueprintCallable, Category="Nyx|FTUE")
	void AdvanceToStep(ENyxFTUEStep NewStep);

	UFUNCTION(BlueprintCallable, Category="Nyx|FTUE")
	void AdvanceToNextStep();

	UFUNCTION(BlueprintCallable, Category="Nyx|FTUE")
	void SetFTUEEnabled(bool bNewEnabled);

	UFUNCTION(BlueprintCallable, Category="Nyx|FTUE|Binding")
	void BindToCoreLoop(UFishingComponent* FishingComponent, UEconomyComponent* EconomyComponent, UDeckComponent* DeckComponent, AStarwell* Starwell);

	UFUNCTION(BlueprintPure, Category="Nyx|FTUE")
	FNyxFTUEStepDefinition GetCurrentStepDefinition() const;

	UFUNCTION(BlueprintPure, Category="Nyx|FTUE")
	bool IsFTUEActive() const;

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

private:
	UPROPERTY(Transient)
	TObjectPtr<UFishingComponent> BoundFishingComponent;

	UPROPERTY(Transient)
	TObjectPtr<UEconomyComponent> BoundEconomyComponent;

	UPROPERTY(Transient)
	TObjectPtr<UDeckComponent> BoundDeckComponent;

	UPROPERTY(Transient)
	TObjectPtr<AStarwell> BoundStarwell;

	void BuildDefaultStepDefinitions();
	void UnbindCoreLoop();
	const FNyxFTUEStepDefinition* FindStepDefinition(ENyxFTUEStep Step) const;
	static ENyxFTUEStep GetNextStep(ENyxFTUEStep Step);
	void AdvanceIfCurrentStepIs(ENyxFTUEStep ExpectedStep, ENyxFTUEStep NewStep);

	UFUNCTION()
	void HandleCastStarted(UFishingComponent* FishingComponent);

	UFUNCTION()
	void HandleReelStarted(UFishingComponent* FishingComponent, UFishDataAsset* Fish);

	UFUNCTION()
	void HandleCatchCompleted(UFishingComponent* FishingComponent, UFishDataAsset* Fish);

	UFUNCTION()
	void HandleCatchOfferedToStarwell(UFishingComponent* FishingComponent, UFishDataAsset* Fish, AStarwell* Starwell, UEconomyComponent* EconomyComponent, int32 EchoScalesGranted);

	UFUNCTION()
	void HandleFishAccepted(AStarwell* Starwell, UFishDataAsset* Fish, int32 EchoScalesGranted, int32 NewOfferingProgress, UEconomyComponent* EconomyComponent);

	UFUNCTION()
	void HandleCardDrawn(UDeckComponent* DeckComponent, UCardDataAsset* Card);

	UFUNCTION()
	void HandleCardPlayed(UDeckComponent* DeckComponent, UCardDataAsset* Card);
};
