// Copyright 2024 Mizunona. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

DECLARE_LOG_CATEGORY_EXTERN(FPSParkourLog, Log, All);

namespace MyCustomCVars
{
	static TAutoConsoleVariable<int32> CvarCustomDebugScreenLog(
		TEXT("ShowCustomDebugScreenLog"),
		true,
		TEXT("Enable or disable custom screen debug log\n")
		TEXT("0: disable log\n")
		TEXT("1: enable log"),
		ECVF_Cheat
	);
}

