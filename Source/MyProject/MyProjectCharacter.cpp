#include "MyProjectCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"

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

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WeaponMesh->SetupAttachment(GetMesh(), TEXT("WeaponSocket"));

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

	if (FollowCamera == nullptr || WeaponMesh == nullptr) return;

	Ammo--;
	OnMuzzleFlash();

	FVector TraceStart = WeaponMesh->GetSocketLocation(TEXT("MuzzleFlashSocket"));
	FVector CameraForward = FollowCamera->GetForwardVector();
	FVector TraceEnd = TraceStart + (CameraForward * WeaponRange);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	bool bHit = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	DrawDebugLine(GetWorld(), TraceStart, bHit ? HitResult.ImpactPoint : TraceEnd, FColor::Green, false, 2.0f, 0, 2.0f);

	if (bHit)
	{
		AActor* HitActor = HitResult.GetActor();
		if (HitActor)
		{
			UE_LOG(LogTemp, Log, TEXT("Hit target: %s"), *HitActor->GetName());
			DrawDebugSphere(GetWorld(), HitResult.ImpactPoint, 15.0f, 12, FColor::Red, false, 2.0f);
			UGameplayStatics::ApplyDamage(HitActor, BaseDamage, GetController(), this, UDamageType::StaticClass());
			OnHitEffect(HitResult.ImpactPoint, HitResult.ImpactNormal);
		}
		else
		{
			OnHitEffect(HitResult.ImpactPoint, HitResult.ImpactNormal);
		}
	}
	else
	{
		OnHitEffect(TraceEnd, -CameraForward);
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
