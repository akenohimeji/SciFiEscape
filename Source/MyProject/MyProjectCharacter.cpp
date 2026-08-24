#include "MyProjectCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h" // Для отрисовки лазерных лучей выстрела

AMyProjectCharacter::AMyProjectCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AMyProjectCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bWantsToSprint && GetCharacterMovement()->Velocity.Size() > 0.0f)
	{
		if (Stamina > 0.0f)
		{
			Stamina -= StaminaDrainRate * DeltaTime;
			if (Stamina <= 0.0f)
			{
				Stamina = 0.0f;
				GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
			}
		}
	}
	else
	{
		if (Stamina < 100.0f)
		{
			Stamina += 10.0f * DeltaTime;
			if (Stamina > 100.0f) Stamina = 100.0f;
		}
	}
}

void AMyProjectCharacter::StartSprint()
{
	bWantsToSprint = true;
	if (Stamina > 0.0f)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintSpeed;
	}
}

void AMyProjectCharacter::StopSprint()
{
	bWantsToSprint = false;
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AMyProjectCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMyProjectCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMyProjectCharacter::StopSprint);

	// Привязка кнопок для боевой системы
	PlayerInputComponent->BindAction("Shoot", IE_Pressed, this, &AMyProjectCharacter::Shoot);
	PlayerInputComponent->BindAction("Reload", IE_Pressed, this, &AMyProjectCharacter::Reload);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyProjectCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyProjectCharacter::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMyProjectCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMyProjectCharacter::LookUpAtRate);

	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &AMyProjectCharacter::OnResetVR);
}

void AMyProjectCharacter::OnResetVR()
{
}

void AMyProjectCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	Jump();
}

void AMyProjectCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	StopJumping();
}

void AMyProjectCharacter::TurnAtRate(float Rate)
{
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMyProjectCharacter::LookUpAtRate(float Rate)
{
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMyProjectCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void AMyProjectCharacter::MoveRight(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);
	}
}

void AMyProjectCharacter::Shoot()
{
	if (Ammo <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("No ammo! Need to reload."));
		return;
	}

	Ammo--;

	// ГАРАНТИРОВАННЫЙ РАСЧЕТ ИЗ КАМЕРЫ ПЕРСОНАЖА
	if (FollowCamera == nullptr) return;

	// Берем точную позицию камеры и её направление вперед
	FVector TraceStart = FollowCamera->GetComponentLocation();
	FVector ForwardVector = FollowCamera->GetForwardVector();

	// Конечная точка луча (Старт + Направление * Дистанцию)
	FVector TraceEnd = TraceStart + (ForwardVector * WeaponRange);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this); // Игнорируем себя, чтобы пуля не застревала в игроке

	// Пускаем луч
	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	// Отрисовка линии для отладки
	// Делаем линию толщиной 5.0f (вместо 1.0f), чтобы её было отлично видно
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false, 3.0f, 0, 5.0f);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			UE_LOG(LogTemp, Log, TEXT("Hit target: %s"), *HitActor->GetName());

			// Рисуем красную сферу в ТОЧНОЙ точке, куда врезалась пуля
			DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 15.0f, 12, FColor::Red, false, 3.0f);

			UGameplayStatics::ApplyDamage(HitActor, 25.0f, GetController(), this, UDamageType::StaticClass());
		}
	}

}

void AMyProjectCharacter::Reload()
{
	if (Ammo < MaxAmmo)
	{
		Ammo = MaxAmmo;
		UE_LOG(LogTemp, Log, TEXT("Weapon Reloaded!"));
	}
}
