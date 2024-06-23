// Copyright 2024 Mizunona. All Rights Reserved.

#include "FPSParkourGame/Public/FPSParkourCharacterComponent.h"

#include "Components/CapsuleComponent.h"
#include "FPSParkourGame/FPSParkourGame.h"
#include "GameFramework/Character.h"
#include "FPSParkourCharacter.h"

#if WITH_EDITOR

float PrintDuration= 2.0f;
#define PRINTS(x) GEngine->AddOnScreenDebugMessage(-1, PrintDuration? PrintDuration: -1.0f, FColor::Blue, x);

#else

#define PRINTS(x)

#endif

#define CAST_DELTA_VECTOR UpdatedComponent->GetRightVector()* GetCapsuleRadius()* 2;

// Sets default values for this component's properties
UFPSParkourCharacterComponent::UFPSParkourCharacterComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	NavAgentProps.bCanCrouch= true;
}

void UFPSParkourCharacterComponent::InitializeComponent()
{
	Super::InitializeComponent();

	FPSParkourCharacterOwner= Cast<AFPSParkourCharacter>(GetOwner());
}

// Called when the game starts
void UFPSParkourCharacterComponent::BeginPlay()
{
	Super::BeginPlay();

	DefaultMaxWalkSpeed= MaxWalkSpeed;
}

// Called every frame
void UFPSParkourCharacterComponent::TickComponent(float DeltaTime, ELevelTick TickType,
												  FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
}

bool UFPSParkourCharacterComponent::IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const
{
	return MovementMode == MOVE_Custom && CustomMovementMode == InCustomMovementMode;
}

bool UFPSParkourCharacterComponent::IsMovementMode(EMovementMode InMovementMode) const
{
	return InMovementMode == MovementMode;
}

bool UFPSParkourCharacterComponent::CanAttemptJump() const
{
	return Super::CanAttemptJump()|| IsWallRunning();
}

bool UFPSParkourCharacterComponent::DoJump(bool bReplayingMoves)
{
	bool bWasCurrentWallRunning= IsWallRunning();
	if(Super::DoJump(bReplayingMoves))
	{
		if(bWasCurrentWallRunning)
		{
			FVector start= UpdatedComponent->GetComponentLocation();
			FVector castDelta= CAST_DELTA_VECTOR;
			FVector end= bWasWallRunIsRight? start+ castDelta: start- castDelta;
			auto params= FPSParkourCharacterOwner->GetIgnoreCharacterParams();
			FHitResult wallHit;
			// TODO Set can wall running wall type
			GetWorld()->LineTraceSingleByProfile(wallHit, start, end, "BlockAll", params);
			Velocity+= wallHit.Normal* WallJumpOffForce;
			
			if(CharacterOwner->GetLocalRole()== ROLE_Authority&& MyCustomCVars::CvarCustomDebugScreenLog.GetValueOnGameThread())
			{
				PRINTS(CharacterOwner.GetName()+ ": End wall running");
			}
		}

		return true;
	}
	return false;
}

float UFPSParkourCharacterComponent::GetMaxSpeed() const
{
	if(IsMovementMode(MOVE_Walking)&& bWasSprint&& !IsCrouching())
	{
		return MaxSprintSpeed;
	}

	if(MovementMode!= MOVE_Custom)
	{
		return Super::GetMaxSpeed(); 
	}

	switch(CustomMovementMode)
	{
	case CMOVE_WallRun:
		return MaxWallRunSpeed;
	default:
		UE_LOG(FPSParkourLog, Fatal, TEXT("FPSParkourCharacterComponent.GetMaxSpeed: Invalid custom movement mode"));
		return -1.0f;
	}
}

float UFPSParkourCharacterComponent::GetMaxBrakingDeceleration() const
{
	if(MovementMode!= MOVE_Custom)
	{
		return Super::GetMaxBrakingDeceleration();
	}

	switch (CustomMovementMode)
	{
	case CMOVE_WallRun:
		return 0.0f;
	default:
		UE_LOG(FPSParkourLog, Fatal, TEXT("FPSParkourCharacterComponent.GetMaxBrakingDeceleration: Invalid custom movement mode"));
		return -1.0f;
	}
}

void UFPSParkourCharacterComponent::UpdateCharacterStateBeforeMovement(float DeltaSeconds)
{
	// Wall run
	if(IsFalling())
	{
		TryWallRun();
	}
	
	Super::UpdateCharacterStateBeforeMovement(DeltaSeconds);
}

#pragma region Input

void UFPSParkourCharacterComponent::SprintPressed()
{
	bWasSprint= true;
}

void UFPSParkourCharacterComponent::SprintReleased()
{
	bWasSprint= false;
}


void UFPSParkourCharacterComponent::ToggleCrouch()
{
	bWantsToCrouch= !bWantsToCrouch;
}

#pragma endregion Input

#pragma region SavedMove

bool UFPSParkourCharacterComponent::FSavedMove_Custom::CanCombineWith(const FSavedMovePtr& NewMove,
	ACharacter* InCharacter, float MaxDelta) const
{
	FSavedMove_Custom* newCustomMove= static_cast<FSavedMove_Custom*>(NewMove.Get());

	if(Saved_bWasSprint!= newCustomMove->Saved_bWasSprint)
	{
		return false;
	}

	if(Saved_bWasWallRun!= newCustomMove->Saved_bWasWallRun)
	{
		return false;
	}
	
	return FSavedMove_Character::CanCombineWith(NewMove, InCharacter, MaxDelta);
}

void UFPSParkourCharacterComponent::FSavedMove_Custom::Clear()
{
	Super::Clear();

	Saved_bWasSprint= 0;
	Saved_bWasWallRun= 0;
}

uint8 UFPSParkourCharacterComponent::FSavedMove_Custom::GetCompressedFlags() const
{
	uint8 result= Super::GetCompressedFlags();
	if(Saved_bWasSprint)
	{
		result |= FSavedMove_Custom::FLAG_Sprint;
	}

	return result;
}

void UFPSParkourCharacterComponent::FSavedMove_Custom::SetMoveFor(ACharacter* C, float InDeltaTime,
	FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData)
{
	Super::SetMoveFor(C, InDeltaTime, NewAccel, ClientData);

	UFPSParkourCharacterComponent* characterMovement= Cast<UFPSParkourCharacterComponent>(C->GetCharacterMovement());
	
	Saved_bWasSprint= characterMovement->bWasSprint;

	Saved_bWasWallRun= characterMovement->bWasWallRunIsRight;
}

void UFPSParkourCharacterComponent::FSavedMove_Custom::PrepMoveFor(ACharacter* C)
{
	Super::PrepMoveFor(C);

	UFPSParkourCharacterComponent* characterMovement= Cast<UFPSParkourCharacterComponent>(C->GetCharacterMovement());

	characterMovement->bWasSprint= Saved_bWasSprint;

	characterMovement->bWasWallRunIsRight= Saved_bWasWallRun;
}

#pragma endregion SavedMove

#pragma region NetworkPredictionDataClient

UFPSParkourCharacterComponent::FNetworkPredictionData_Client_Custom::FNetworkPredictionData_Client_Custom(const UCharacterMovementComponent& ClientMovement): Super(ClientMovement)
{
}

FSavedMovePtr UFPSParkourCharacterComponent::FNetworkPredictionData_Client_Custom::AllocateNewMove()
{
	return FSavedMovePtr(new FSavedMove_Custom());
}

#pragma endregion NetworkPredictionDataClient

FNetworkPredictionData_Client* UFPSParkourCharacterComponent::GetPredictionData_Client() const
{
	check(PawnOwner!= nullptr);

	if(!ClientPredictionData)
	{
		UFPSParkourCharacterComponent* cc= const_cast<UFPSParkourCharacterComponent*>(this);
		cc->ClientPredictionData= new FNetworkPredictionData_Client_Custom(*this);
		cc->ClientPredictionData->MaxSmoothNetUpdateDist= 92.0f;
		cc->ClientPredictionData->NoSmoothNetUpdateDist= 140.0f;
	}
	
	return ClientPredictionData;
}

void UFPSParkourCharacterComponent::UpdateFromCompressedFlags(uint8 Flags)
{
	Super::UpdateFromCompressedFlags(Flags);

	bWasSprint= (Flags& FSavedMove_Custom::FLAG_Sprint)!= 0;
}

void UFPSParkourCharacterComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);

	if(MovementMode== MOVE_Walking)
	{
		if(bWasSprint)
		{
			MaxWalkSpeed= MaxSprintSpeed;
		}else
		{
			MaxWalkSpeed= DefaultMaxWalkSpeed;
		}
	}
}

void UFPSParkourCharacterComponent::PhysCustom(float deltaTime, int32 Iterations)
{
	Super::PhysCustom(deltaTime, Iterations);

	switch (CustomMovementMode)
	{
	case CMOVE_WallRun:
		PhysWallRun(deltaTime, Iterations);
		break;
	default:
		UE_LOG(FPSParkourLog, Fatal, TEXT("FPSParkourCharacterComponent.PhysCustom: Invalid custom movement mode"));
	}
}

void UFPSParkourCharacterComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);

	if(IsWallRunning()&& GetOwnerRole()== ROLE_SimulatedProxy)
	{
		FVector start= UpdatedComponent->GetComponentLocation();
		FVector end= start+ CAST_DELTA_VECTOR;
		auto params= FPSParkourCharacterOwner->GetIgnoreCharacterParams();
		FHitResult wallHit;
		// TODO Set can wall running wall type
		bWasWallRunIsRight= GetWorld()->LineTraceSingleByProfile(wallHit, start, end, "BlockAll", params);
	}
}

#pragma region WallRun

void UFPSParkourCharacterComponent::TryWallRun()
{
	if(!IsFalling())
	{
		return;
	}
	if(Velocity.Z< -MaxVerticalWallRunSpeed)
	{
		return;
	}

	FVector start= UpdatedComponent->GetComponentLocation();
	FVector leftEnd= start- CAST_DELTA_VECTOR;
	FVector rightEnd= start+ CAST_DELTA_VECTOR;
	auto params= FPSParkourCharacterOwner->GetIgnoreCharacterParams();
	FHitResult hit;

	// TODO Set ground type
	// Check if the player is above the ground + MinWallRunHeight
	bool bIsTracedGround= GetWorld()->LineTraceSingleByProfile(hit, start, start+ FVector::DownVector* (GetCapsuleHalfHeight()+ MinWallRunHeight), "BlockAll", params);
	if(bIsTracedGround)
	{
		return;		
	}

	hit.Init();
	
	// TODO Set can wall running wall type
	// Left cast
	GetWorld()->LineTraceSingleByProfile(hit, start, leftEnd, "BlockAll", params);
	if(hit.IsValidBlockingHit()&& (Velocity| hit.Normal)< 0)
	{
		bWasWallRunIsRight= false;
	}
	else
	{
		hit.Init();
		// TODO Set can wall running wall type
		// Right cast
		GetWorld()->LineTraceSingleByProfile(hit, start, rightEnd, "BlockAll", params);
		if(hit.IsValidBlockingHit()&& (Velocity| hit.Normal)< 0)
		{
			bWasWallRunIsRight= true;
		}
		else
		{
			return;
		}
	}
	
	FVector projectedVelocity= FVector::VectorPlaneProject(Velocity, hit.Normal);
	if(projectedVelocity.SizeSquared2D()< pow(MinWallRunSpeed, 2))
	{
		return;
	}
	
	// Calculate wall running velocity
	Velocity= projectedVelocity;
	Velocity.Z= FMath::Clamp(Velocity.Z, 0.0f, MaxVerticalWallRunSpeed);
	SetMovementMode(MOVE_Custom, CMOVE_WallRun);
	WallRunMovementTriggerDelegate.Broadcast(true);

	if(CharacterOwner->GetLocalRole()== ROLE_Authority&& MyCustomCVars::CvarCustomDebugScreenLog.GetValueOnGameThread())
	{
		PRINTS(CharacterOwner.GetName()+ ": Start wall running");
	}
}

void UFPSParkourCharacterComponent::PhysWallRun(float DeltaTime, int32 Iterations)
{
	if(DeltaTime< MIN_TICK_TIME)
	{
		return;
	}
	if(!CharacterOwner|| (!CharacterOwner->Controller&& !bRunPhysicsWithNoController&& !HasAnimRootMotion()&& !CurrentRootMotion.HasOverrideVelocity()&& (CharacterOwner->GetLocalRole()!= ROLE_SimulatedProxy)))
	{
		Acceleration= FVector::ZeroVector;
		Velocity= FVector::ZeroVector;
		return;
	}

	bJustTeleported= false;
	float remainingTime= DeltaTime;

	// Perform move
	while((remainingTime>= MIN_TICK_TIME)&& (Iterations< MaxSimulationIterations)&& CharacterOwner&& (CharacterOwner->Controller|| bRunPhysicsWithNoController|| (CharacterOwner->GetLocalRole()!= ROLE_SimulatedProxy)))
	{
		Iterations++;
		bJustTeleported= false;
		const float timeTick= GetSimulationTimeStep(remainingTime, Iterations);
		remainingTime-= timeTick;
		const FVector oldLocation= UpdatedComponent->GetComponentLocation();
		FVector start= oldLocation;
		FVector castDelta= CAST_DELTA_VECTOR;
		FVector end= bWasWallRunIsRight? start+ castDelta: start- castDelta;
		auto params= FPSParkourCharacterOwner->GetIgnoreCharacterParams();
		FHitResult wallHit;
		// TODO Set can wall running wall type
		GetWorld()->LineTraceSingleByProfile(wallHit, start, end, "BlockAll", params);

		float sinPullAwayAngle= FMath::Sin(FMath::DegreesToRadians(WallRunPullAwayAngle));
		bool bWantsToPullAway= wallHit.IsValidBlockingHit()&& !Acceleration.IsNearlyZero()&& (Acceleration.GetSafeNormal()| wallHit.Normal)> sinPullAwayAngle;
		if(!wallHit.IsValidBlockingHit()|| bWantsToPullAway)
		{
			if(CharacterOwner->GetLocalRole()== ROLE_Authority&& MyCustomCVars::CvarCustomDebugScreenLog.GetValueOnGameThread())
			{
				PRINTS(CharacterOwner.GetName()+ ": End wall running");
			}
			SetMovementMode(MOVE_Falling);
			WallRunMovementTriggerDelegate.Broadcast(false);
			StartNewPhysics(remainingTime, Iterations);
			return;
		}

		// Clamp acceleration
		Acceleration= FVector::VectorPlaneProject(Acceleration, wallHit.Normal);
		Acceleration.Z= 0.0f;

		// Apply acceleration
		CalcVelocity(timeTick, 0.0f, false, GetMaxBrakingDeceleration());
		Velocity= FVector::VectorPlaneProject(Velocity, wallHit.Normal);
		float tangentAccel= Acceleration.GetSafeNormal()| Velocity.GetSafeNormal2D();
		bool bVelocityUp= Velocity.Z> 0.0f;
		Velocity.Z+= GetGravityZ()* WallRunGravityScaleCurve->GetFloatValue(bVelocityUp? 0.0f: tangentAccel)* timeTick;
		if(Velocity.SizeSquared2D()< pow(MinWallRunSpeed, 2)|| Velocity.Z< -MaxVerticalWallRunSpeed)
		{
			if(CharacterOwner->GetLocalRole()== ROLE_Authority&& MyCustomCVars::CvarCustomDebugScreenLog.GetValueOnGameThread())
			{
				PRINTS(CharacterOwner.GetName()+ ": End wall running");
			}
			SetMovementMode(MOVE_Falling);
			WallRunMovementTriggerDelegate.Broadcast(false);
			StartNewPhysics(remainingTime, Iterations);
			return;
		}

		// Calculate move parameters
		// dx = v * dt
		const FVector deltaVector= timeTick* Velocity;
		const bool bZeroDelta= deltaVector.IsNearlyZero();
		if(bZeroDelta)
		{
			remainingTime= 0.0f;
		}else
		{
			FHitResult hit;
			SafeMoveUpdatedComponent(deltaVector, UpdatedComponent->GetComponentQuat(), true, hit);
			FVector wallAttractionDeltaVector= -wallHit.Normal* WallAttractionForce* timeTick;
			SafeMoveUpdatedComponent(wallAttractionDeltaVector, UpdatedComponent->GetComponentQuat(), true, hit);
		}
		if(UpdatedComponent->GetComponentLocation()== oldLocation)
		{
			remainingTime= 0.0f;
			break;
		}

		// v = dx / dt
		Velocity= (UpdatedComponent->GetComponentLocation()- oldLocation)/ timeTick;
	}

	FVector start= UpdatedComponent->GetComponentLocation();
	FVector castDelta= CAST_DELTA_VECTOR;
	FVector end= bWasWallRunIsRight? start+ castDelta: start- castDelta;
	auto params= FPSParkourCharacterOwner->GetIgnoreCharacterParams();
	FHitResult floorHit, wallHit;
	GetWorld()->LineTraceSingleByProfile(wallHit, start, end, "BlockAll", params);
	GetWorld()->LineTraceSingleByProfile(floorHit, start, start+ FVector::DownVector* (GetCapsuleHalfHeight()* 0.5f), "BlockAll", params);
	if(floorHit.IsValidBlockingHit()|| !wallHit.IsValidBlockingHit()|| Velocity.SizeSquared2D()< pow(MinWallRunSpeed, 2))
	{
		if(CharacterOwner->GetLocalRole()== ROLE_Authority&& MyCustomCVars::CvarCustomDebugScreenLog.GetValueOnGameThread())
		{
			PRINTS(CharacterOwner.GetName()+ ": End wall running");
		}
		SetMovementMode(MOVE_Falling);
		WallRunMovementTriggerDelegate.Broadcast(false);
	}
}

#pragma endregion  WallRun

float UFPSParkourCharacterComponent::GetCapsuleRadius() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleRadius();
}

float UFPSParkourCharacterComponent::GetCapsuleHalfHeight() const
{
	return CharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
}

