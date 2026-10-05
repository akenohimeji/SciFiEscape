#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MyProjectCharacter.generated.h"

UCLASS(config = Game)
class AMyProjectCharacter : public ACharacter
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Компонент для 3D-модели автомата/револьвера */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Weapon, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* WeaponMesh;

public:
	AMyProjectCharacter();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	// Добавили EditAnywhere, чтобы переменные отображались в Details внутри Блупринта
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Config, Category = "Attributes")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Config, Category = "Attributes")
	float Stamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Config, Category = "Attributes")
	int32 Ammo = 6;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Config, Category = "Attributes")
	int32 MaxAmmo = 6;

	/** Встроенная функция Unreal Engine для получения урона */
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

protected:
	void OnResetVR();
	void MoveForward(float Value);
	void MoveRight(float Value);
	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);

protected:
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaTime) override;

	// Функции передвижения
	void StartSprint();
	void StopSprint();

	// Функции боевой системы
	void Shoot();
	void Reload();

	/** Событие для вызова эффектов попадания в Blueprint */
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Effects")
	void OnHitEffect(FVector ImpactPoint, FVector ImpactNormal);

	/** Событие для вызова вспышки выстрела in Blueprint */
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Effects")
	void OnMuzzleFlash();

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Combat")
	float WeaponRange = 5000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Combat")
	float BaseDamage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Movement")
	float WalkSpeed = 600.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Movement")
	float SprintSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Config, Category = "Movement")
	float StaminaDrainRate = 20.0f;

	bool bWantsToSprint = false;

	/** Ссылка на класс виджета экрана смерти, которую мы выберем в Блупринте */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "UI")
	TSubclassOf<class UUserWidget> LoseScreenWidgetClass;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	FORCEINLINE class USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
};
