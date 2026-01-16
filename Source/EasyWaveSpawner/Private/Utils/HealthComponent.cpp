#include "Utils/HealthComponent.h"

#include "Core/WaveManagerComponent.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"


UHealthComponent::UHealthComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	CurrentHealth = MaxHealth;
}

void UHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	// 注册伤害监听
	if (AActor* MyOwner = GetOwner())
	{
		MyOwner->OnTakeAnyDamage.AddDynamic(this, &UHealthComponent::HandleTakeAnyDamage);
	}
	
}

void UHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType,
	class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage <= 0.0f || bIsDead) return;

	CurrentHealth = FMath::Clamp(CurrentHealth - Damage, 0.0f, MaxHealth);
	OnHealthChanged.Broadcast(CurrentHealth, MaxHealth, Damage, DamageCauser);

	if (CurrentHealth <= 0.0f)
	{
		bIsDead = true;
		OnDeath.Broadcast(DamagedActor);

		AActor* GameModeActor = UGameplayStatics::GetGameMode(GetWorld());
		if (GameModeActor)
		{
			UWaveManagerComponent* WaveManager = GameModeActor->FindComponentByClass<UWaveManagerComponent>();
			if (WaveManager)
			{
				WaveManager->ReportEnemyDeath(DamagedActor);
			}
		}
	}
}


