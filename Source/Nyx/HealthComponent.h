#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"

class UHealthComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnHealthChangedSignature, UHealthComponent*, HealthComponent, float, CurrentHealth, float, MaxHealth, float, HealthDelta, AActor*, InstigatorActor);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeathSignature, UHealthComponent*, HealthComponent, AActor*, DeadActor, AActor*, KillerActor);

UCLASS(ClassGroup=(Health), Blueprintable, meta=(BlueprintSpawnableComponent))
class NYX_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHealthComponent();

	// Maximum health for this actor. Change this on the server so clients receive the replicated value.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_MaxHealth, Category="Health", meta=(ClampMin="0.0", UIMin="0.0"))
	float MaxHealth;

	// Server-owned current health. Clients receive updates through replication and should treat this as read-only.
	UPROPERTY(VisibleInstanceOnly, BlueprintReadOnly, ReplicatedUsing=OnRep_CurrentHealth, Category="Health")
	float CurrentHealth;

	// Broadcast after health or max health changes. HealthDelta is CurrentHealth minus the previous CurrentHealth.
	UPROPERTY(BlueprintAssignable, Category="Health")
	FOnHealthChangedSignature OnHealthChanged;

	// Broadcast once when health crosses from above zero to zero.
	UPROPERTY(BlueprintAssignable, Category="Health")
	FOnDeathSignature OnDeath;

	// Applies damage on the server. In network games, the owning actor must replicate for component RPCs/replication to work.
	UFUNCTION(BlueprintCallable, Category="Health")
	void Damage(float Amount, AActor* InstigatorActor = nullptr);

	// Applies healing on the server. Healing from zero is allowed so gameplay can support revives.
	UFUNCTION(BlueprintCallable, Category="Health")
	void Heal(float Amount, AActor* InstigatorActor = nullptr);

	UFUNCTION(BlueprintPure, Category="Health")
	float GetHealthPercent() const;

	UFUNCTION(BlueprintPure, Category="Health")
	bool IsDead() const;

protected:
	virtual void BeginPlay() override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(Server, Reliable)
	void ServerDamage(float Amount, AActor* InstigatorActor);

	UFUNCTION(Server, Reliable)
	void ServerHeal(float Amount, AActor* InstigatorActor);

	UFUNCTION()
	void OnRep_CurrentHealth(float OldCurrentHealth);

	UFUNCTION()
	void OnRep_MaxHealth(float OldMaxHealth);

private:
	bool IsAuthoritative() const;
	void ApplyHealthDelta(float HealthDelta, AActor* InstigatorActor);
	void BroadcastHealthChanged(float OldHealth, AActor* InstigatorActor);
};
