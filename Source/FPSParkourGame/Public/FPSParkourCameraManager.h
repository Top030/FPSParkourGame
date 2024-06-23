// Copyright 2024 Mizunona. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/PlayerCameraManager.h"
#include "FPSParkourCameraManager.generated.h"

UCLASS()
class FPSPARKOURGAME_API AFPSParkourCameraManager : public APlayerCameraManager
{
	GENERATED_BODY()

public:
	AFPSParkourCameraManager();

	virtual void UpdateViewTarget(FTViewTarget& OutVT, float DeltaTime) override;
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Movement", meta= (AllowPrivateAccess= "true"))
	float CrouchBlendMaxTime= 0.25f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Movement", meta= (AllowPrivateAccess= "true"))
	float WallRunBlendMaxTime= 0.1f;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Movement", meta= (AllowPrivateAccess= "true", ClampMin= "0", UIMin= "0", ForceUnits="degrees"))
	float WallRunCameraAngle= 15.0f;
	
private:
	float CrouchBlendTime;

	float WallRunBlendTime;
};
