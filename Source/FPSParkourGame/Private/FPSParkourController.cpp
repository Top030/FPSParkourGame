// Copyright 2024 Mizunona. All Rights Reserved.

#include "FPSParkourController.h"

#include "FPSParkourCameraManager.h"

AFPSParkourController::AFPSParkourController()
{
	PlayerCameraManagerClass= AFPSParkourCameraManager::StaticClass();
}
