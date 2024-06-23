// Copyright 2024 Mizunona. All Rights Reserved.

#include "FPSParkourGame/Public/FPSParkourCharacter.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "FPSParkourGame/Public/FPSParkourCharacterComponent.h"

// Sets default values
AFPSParkourCharacter::AFPSParkourCharacter(const FObjectInitializer& ObjectInitializer): Super(ObjectInitializer.SetDefaultSubobjectClass<UFPSParkourCharacterComponent>(CharacterMovementComponentName))
{
	PrimaryActorTick.bCanEverTick = true;

	FPSParkourCharacterComponent= Cast<UFPSParkourCharacterComponent>(GetCharacterMovement());
	
	// Setup camera component
	FPSCameraComponent= CreateDefaultSubobject<UCameraComponent>(TEXT("FPSCamera"));
	FPSCameraComponent->SetupAttachment(GetCapsuleComponent());
	FPSCameraComponent->bUsePawnControlRotation= true;
}

// Called when the game starts or when spawned
void AFPSParkourCharacter::BeginPlay()
{
	Super::BeginPlay();
	
	if(const APlayerController* playerController= Cast<APlayerController>(Controller))
	{
		if(UEnhancedInputLocalPlayerSubsystem* inputSubsystem= ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(playerController->GetLocalPlayer()))
		{
			inputSubsystem->AddMappingContext(DefaultInputMappingContext, 0);
		}
	}
}

// Called every frame
void AFPSParkourCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

// Called to bind functionality to input
void AFPSParkourCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	//Setup player action bindings
	if(UEnhancedInputComponent* eic= CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		eic->BindAction(IA_Move, ETriggerEvent::Triggered, this, &AFPSParkourCharacter::CharacterMovement);
		eic->BindAction(IA_Look, ETriggerEvent::Triggered, this, &AFPSParkourCharacter::CharacterLook);
		eic->BindAction(IA_Jump, ETriggerEvent::Started, this, &ACharacter::Jump);
		eic->BindAction(IA_Jump, ETriggerEvent::Completed, this, &ACharacter::StopJumping);
		eic->BindAction(IA_Sprint, ETriggerEvent::Started, this, &AFPSParkourCharacter::CharacterSprint);
		eic->BindAction(IA_Sprint, ETriggerEvent::Completed, this, &AFPSParkourCharacter::CharacterSprint);
		eic->BindAction(IA_Crouch, ETriggerEvent::Started, this, &AFPSParkourCharacter::CharacterCrouch);
	}
}

FCollisionQueryParams AFPSParkourCharacter::GetIgnoreCharacterParams() const
{
	FCollisionQueryParams params;

	TArray<AActor*> characterChildren;
	GetAllChildActors(characterChildren);
	params.AddIgnoredActors(characterChildren);
	params.AddIgnoredActor(this);

	return params;
}

#pragma region Movement

void AFPSParkourCharacter::CharacterMovement(const FInputActionValue& ActionValue)
{
	FVector2D movementVector= ActionValue.Get<FVector2D>();

	if(Controller)
	{
		AddMovementInput(GetActorForwardVector(), movementVector.Y);
		AddMovementInput(GetActorRightVector(), movementVector.X);
	}
}

void AFPSParkourCharacter::CharacterLook(const FInputActionValue& ActionValue)
{
	FVector2D lookAxisVector= ActionValue.Get<FVector2D>();

	if(Controller)
	{
		AddControllerYawInput(lookAxisVector.X);
		AddControllerPitchInput(lookAxisVector.Y);
	}
}

void AFPSParkourCharacter::CharacterSprint(const FInputActionValue& ActionValue)
{
	check(FPSParkourCharacterComponent!= nullptr);

	bool bSprint= ActionValue.Get<bool>();
	if(bSprint)
	{
		FPSParkourCharacterComponent->SprintPressed();
	}else
	{
		FPSParkourCharacterComponent->SprintReleased();
	}
}

void AFPSParkourCharacter::CharacterCrouch(const FInputActionValue& ActionValue)
{
	check(FPSParkourCharacterComponent!= nullptr);

	FPSParkourCharacterComponent->ToggleCrouch();
}

UCameraComponent* AFPSParkourCharacter::GetCameraComponent() const
{
	return FPSCameraComponent;
}

#pragma endregion Movement
