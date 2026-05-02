#include "HealthComponent.h"

#include "GameFramework/Actor.h"
#include "Net/UnrealNetwork.h"

UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);

	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	if (IsAuthoritative())
	{
		MaxHealth = FMath::Max(0.0f, MaxHealth);
		CurrentHealth = MaxHealth;
	}
}

void UHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UHealthComponent, MaxHealth);
	DOREPLIFETIME(UHealthComponent, CurrentHealth);
}

void UHealthComponent::Damage(float Amount, AActor* InstigatorActor)
{
	if (Amount <= 0.0f || IsDead())
	{
		return;
	}

	if (!IsAuthoritative())
	{
		ServerDamage(Amount, InstigatorActor);
		return;
	}

	ApplyHealthDelta(-Amount, InstigatorActor);
}

void UHealthComponent::Heal(float Amount, AActor* InstigatorActor)
{
	if (Amount <= 0.0f)
	{
		return;
	}

	if (!IsAuthoritative())
	{
		ServerHeal(Amount, InstigatorActor);
		return;
	}

	ApplyHealthDelta(Amount, InstigatorActor);
}

float UHealthComponent::GetHealthPercent() const
{
	return MaxHealth > KINDA_SMALL_NUMBER ? CurrentHealth / MaxHealth : 0.0f;
}

bool UHealthComponent::IsDead() const
{
	return CurrentHealth <= 0.0f;
}

void UHealthComponent::ServerDamage_Implementation(float Amount, AActor* InstigatorActor)
{
	Damage(Amount, InstigatorActor);
}

void UHealthComponent::ServerHeal_Implementation(float Amount, AActor* InstigatorActor)
{
	Heal(Amount, InstigatorActor);
}

void UHealthComponent::OnRep_CurrentHealth(float OldCurrentHealth)
{
	BroadcastHealthChanged(OldCurrentHealth, nullptr);

	if (OldCurrentHealth > 0.0f && IsDead())
	{
		OnDeath.Broadcast(this, GetOwner(), nullptr);
	}
}

void UHealthComponent::OnRep_MaxHealth(float OldMaxHealth)
{
	if (!FMath::IsNearlyEqual(OldMaxHealth, MaxHealth))
	{
		OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, 0.0f, nullptr);
	}
}

bool UHealthComponent::IsAuthoritative() const
{
	const AActor* Owner = GetOwner();
	return Owner == nullptr || Owner->HasAuthority();
}

void UHealthComponent::ApplyHealthDelta(float HealthDelta, AActor* InstigatorActor)
{
	if (!IsAuthoritative())
	{
		return;
	}

	MaxHealth = FMath::Max(0.0f, MaxHealth);

	const float OldHealth = CurrentHealth;
	const bool bWasDead = IsDead();

	CurrentHealth = FMath::Clamp(CurrentHealth + HealthDelta, 0.0f, MaxHealth);

	if (FMath::IsNearlyEqual(OldHealth, CurrentHealth))
	{
		return;
	}

	BroadcastHealthChanged(OldHealth, InstigatorActor);

	if (!bWasDead && IsDead())
	{
		OnDeath.Broadcast(this, GetOwner(), InstigatorActor);
	}
}

void UHealthComponent::BroadcastHealthChanged(float OldHealth, AActor* InstigatorActor)
{
	const float HealthDelta = CurrentHealth - OldHealth;
	OnHealthChanged.Broadcast(this, CurrentHealth, MaxHealth, HealthDelta, InstigatorActor);
}
