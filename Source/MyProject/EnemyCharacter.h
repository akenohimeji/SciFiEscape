#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnemyCharacter.generated.h"

UCLASS()
class MYPROJECT_API AEnemyCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AEnemyCharacter();

protected:
	virtual void BeginPlay() override;

	// Здоровье врага
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Attributes")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Attributes")
	float MaxHealth = 100.0f;

public:
	// Встроенная функция Unreal Engine для получения урона
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	// Функция смерти врага
	void Die();
};
