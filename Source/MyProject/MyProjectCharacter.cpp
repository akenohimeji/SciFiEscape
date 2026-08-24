#include "MyProjectCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"

AMyProjectCharacter::AMyProjectCharacter()
{
	// Включаем Tick, чтобы работало ежекадное уменьшение стамины
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Устанавливаем стартовую скорость ходьбы из нашей переменной
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;
}

void AMyProjectCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Проверяем логику спринта каждый кадр
	if (bWantsToSprint && GetCharacterMovement()->Velocity.Size() > 0.0f)
	{
		// Если бежим и есть стамина — тратим её
		if (Stamina > 0.0f)
		{
			Stamina -= StaminaDrainRate * DeltaTime;
			if (Stamina <= 0.0f)
			{
				Stamina = 0.0f;
				GetCharacterMovement()->MaxWalkSpeed = WalkSpeed; // Замедляемся, если стамина кончилась
			}
		}
	}
	else
	{
		// Если не спринтуем — плавно восстанавливаем стамину (например, +10 в секунду)
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
	// Разрешаем разгон только если стамины больше нуля
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

	// ПРИВЯЗКА КНОПКИ СПРИНТА (в настройках проекта Unreal Engine экшен должен называться "Sprint")
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMyProjectCharacter::StartSprint);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMyProjectCharacter::StopSprint);

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
