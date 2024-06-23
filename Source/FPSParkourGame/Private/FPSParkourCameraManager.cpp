// Copyright 2024 Mizunona. All Rights Reserved.

#include "FPSParkourCameraManager.h"

#include "FPSParkourCharacter.h"
#include "FPSParkourCharacterComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"

AFPSParkourCameraManager::AFPSParkourCameraManager()
{
	ViewPitchMax= 80.0f;
	ViewPitchMin= -80.0f;
}

void AFPSParkourCameraManager::UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime)
{
	Super::UpdateViewTarget(OutVT, DeltaTime);

	if(const AFPSParkourCharacter* character= Cast<AFPSParkourCharacter>(GetOwningPlayerController()->GetPawn()))
	{
		UFPSParkourCharacterComponent* cc= character->GetFPSParkourCharacterMovement();

		// Crouch
		FVector targetCrouchOffset= FVector(0, 0, cc->GetCrouchedHalfHeight()- character->GetClass()->GetDefaultObject<ACharacter>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight());
		FVector crouchOffset= FMath::Lerp(FVector::ZeroVector, targetCrouchOffset, FMath::Clamp(CrouchBlendTime/ CrouchBlendMaxTime, 0.0f, 1.0f));

		if(cc->IsCrouching())
		{
			CrouchBlendTime= FMath::Clamp(CrouchBlendTime+ DeltaTime, 0.0f, CrouchBlendMaxTime);
			crouchOffset-= targetCrouchOffset;
		}else
		{
			CrouchBlendTime= FMath::Clamp(CrouchBlendTime- DeltaTime, 0.0f, CrouchBlendMaxTime);
		}
		
		if(cc->IsMovingOnGround())
		{
			OutVT.POV.Location+= crouchOffset;
		}

		// Wall run
		FRotator currentRotator=GetOwningPlayerController()->GetControlRotation();
		float targetWallRunRollOffset= 0.0f;

		if(cc->WallRunningIsRight())
		{
			targetWallRunRollOffset= WallRunCameraAngle* -1.0f;
		}else
		{
			targetWallRunRollOffset= WallRunCameraAngle;
		}
		
		float wallRunRollOffset= FMath::Lerp(0.0f, targetWallRunRollOffset, FMath::Clamp(WallRunBlendTime/ WallRunBlendMaxTime, 0.0f, 1.0f));
		if(cc->IsWallRunning())
		{
			WallRunBlendTime= FMath::Clamp(WallRunBlendTime+ DeltaTime, 0.0f, WallRunBlendMaxTime);
		}else
		{
			WallRunBlendTime= FMath::Clamp(WallRunBlendTime- DeltaTime, 0.0f, WallRunBlendMaxTime);
		}

		if(character->GetCameraComponent()->bUsePawnControlRotation)
		{
			FRotator targetRotator= FRotator(currentRotator.Pitch, currentRotator.Yaw, wallRunRollOffset);
			GetOwningPlayerController()->SetControlRotation(targetRotator);
		}
	}
}

void AFPSParkourCameraManager::BeginPlay()
{
	Super::BeginPlay();
}


