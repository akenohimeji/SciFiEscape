#include "EnemyCharacter.h"
#include "Components/CapsuleComponent.h"

AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	// Настраиваем коллизии капсулы, чтобы по врагу регистрировались выстрелы LineTrace
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	Health = MaxHealth;
}

float AEnemyCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float DamageApplied = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if (Health <= 0.0f) return 0.0f; // Уже мертв

	Health -= DamageApplied;
	UE_LOG(LogTemp, Log, TEXT("Enemy HP: %f"), Health);

	if (Health <= 0.0f)
	{
		Die();
	}

	return DamageApplied;
}

void AEnemyCharacter::Die()
{
	UE_LOG(LogTemp, Warning, TEXT("Enemy is DEAD!"));

	// Простая логика смерти: отключаем коллизии и уничтожаем актера
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Destroy();
}
