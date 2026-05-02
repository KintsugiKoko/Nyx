#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "NyxGameplayValidation.generated.h"

USTRUCT(BlueprintType)
struct FNyxGameplayValidationResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Nyx|Validation")
	bool bPassed = true;

	UPROPERTY(BlueprintReadOnly, Category="Nyx|Validation")
	FString Summary;

	UPROPERTY(BlueprintReadOnly, Category="Nyx|Validation")
	TArray<FString> Failures;

	void AddFailure(const FString& FailureMessage)
	{
		bPassed = false;
		Failures.Add(FailureMessage);
	}
};

UCLASS()
class NYX_API UNyxGameplayValidationLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="Nyx|Validation")
	static FNyxGameplayValidationResult ValidateEconomySystem();

	UFUNCTION(BlueprintCallable, Category="Nyx|Validation")
	static FNyxGameplayValidationResult ValidateDeckSystem();

	UFUNCTION(BlueprintCallable, Category="Nyx|Validation")
	static FNyxGameplayValidationResult ValidateFishingSystem();

	UFUNCTION(BlueprintCallable, Category="Nyx|Validation", meta=(WorldContext="WorldContextObject"))
	static FNyxGameplayValidationResult ValidateStarwellSystem(UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category="Nyx|Validation")
	static FNyxGameplayValidationResult ValidateSaveGameSystem();

	UFUNCTION(BlueprintCallable, Category="Nyx|Validation", meta=(WorldContext="WorldContextObject"))
	static FNyxGameplayValidationResult RunAllGameplayValidations(UObject* WorldContextObject);

private:
	static void MergeValidationResult(FNyxGameplayValidationResult& Target, const FNyxGameplayValidationResult& Source);
	static void FinalizeValidationResult(FNyxGameplayValidationResult& Result, const FString& SystemName);
};
