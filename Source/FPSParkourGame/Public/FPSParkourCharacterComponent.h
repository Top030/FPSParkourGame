// Copyright 2024 Mizunona. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "FPSParkourCharacterComponent.generated.h"

UENUM(BlueprintType)
enum ECustomMovementMode
{
	CMOVE_None	UMETA(Hidden),
	CMOVE_WallRun	UMETA(DisplayName= "Wall Run"),
	CMOVE_Max	UMETA(Hidden)
};

class UCurveFloat;
class AFPSParkourCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWallRunMovementDelegate, bool, bIsInWallRunState);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class FPSPARKOURGAME_API UFPSParkourCharacterComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()
	
public:
	// Sets default values for this component's properties
	UFPSParkourCharacterComponent();

	virtual void InitializeComponent() override;

	// Sprint
	UFUNCTION(BlueprintCallable, Category= "Movement")
	void SprintPressed();
	UFUNCTION(BlueprintCallable, Category= "Movement")
	void SprintReleased();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Character Movement: Sprinting", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxSprintSpeed= 900.0f;

	// Crouch
	UFUNCTION(BlueprintCallable, Category= "Movement")
	void ToggleCrouch();

	// Wall run
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Character Movement: Wall Running", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MinWallRunSpeed= 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Character Movement: Wall Running", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxWallRunSpeed= 800.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Character Movement: Wall Running", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MaxVerticalWallRunSpeed= 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Character Movement: Wall Running", meta=(ClampMin="0", UIMin="0", ClampMax="90", UIMax="90", ForceUnits="degrees"))
	float WallRunPullAwayAngle= 75.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Character Movement: Wall Running", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float WallAttractionForce= 200.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Character Movement: Wall Running", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float MinWallRunHeight= 50.0f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Character Movement: Wall Running")
	UCurveFloat* WallRunGravityScaleCurve;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category= "Character Movement: Wall Running", meta=(ClampMin="0", UIMin="0", ForceUnits="cm/s"))
	float WallJumpOffForce= 300.0f;
	UFUNCTION(BlueprintPure, Category= "Movement")
	bool IsWallRunning() const { return IsCustomMovementMode(CMOVE_WallRun); }
	UFUNCTION(BlueprintPure, Category= "Movement")
	bool WallRunningIsRight() const { return bWasWallRunIsRight; }
	UPROPERTY(BlueprintAssignable)
	FWallRunMovementDelegate WallRunMovementTriggerDelegate;
	
	UFUNCTION(BlueprintPure, Category= "Movement")
	bool IsCustomMovementMode(ECustomMovementMode InCustomMovementMode) const;
	UFUNCTION(BlueprintPure, Category= "Movement")
	bool IsMovementMode(EMovementMode InMovementMode) const;

	UPROPERTY(Transient)
	AFPSParkourCharacter* FPSParkourCharacterOwner;

	virtual bool CanAttemptJump() const override;
	virtual bool DoJump(bool bReplayingMoves) override;
	virtual float GetMaxSpeed() const override;
	virtual float GetMaxBrakingDeceleration() const override;
	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
							   FActorComponentTickFunction* ThisTickFunction) override;

#pragma region Net
	class FSavedMove_Custom: public FSavedMove_Character
	{
	public:
		enum CompressedFlags
		{
			FLAG_Sprint = 0x10,
			FLAG_Custom_1 = 0x20,
			FLAG_Custom_2 = 0x40,
			FLAG_Custom_3 = 0x80,
		};
		
		typedef FSavedMove_Character Super;
		
		uint8 Saved_bWasSprint:1;

		uint8 Saved_bWasWallRun:1;
		
		virtual bool CanCombineWith(const FSavedMovePtr& NewMove, ACharacter* InCharacter, float MaxDelta) const override;
		virtual void Clear() override;
		virtual uint8 GetCompressedFlags() const override;
		virtual void SetMoveFor(ACharacter* C, float InDeltaTime, FVector const& NewAccel, FNetworkPredictionData_Client_Character& ClientData) override;
		virtual void PrepMoveFor(ACharacter* C) override;
	};

	class FNetworkPredictionData_Client_Custom: public FNetworkPredictionData_Client_Character
	{
	public:

		typedef FNetworkPredictionData_Client_Character Super;
		FNetworkPredictionData_Client_Custom(const UCharacterMovementComponent& ClientMovement);

		virtual FSavedMovePtr AllocateNewMove() override;
	};

	virtual FNetworkPredictionData_Client* GetPredictionData_Client() const override;

#pragma endregion Net
	
protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	virtual void UpdateFromCompressedFlags(uint8 Flags) override;

	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;
	virtual void PhysCustom(float deltaTime, int32 Iterations) override;
	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

private:
	// Sprint
	bool bWasSprint;
	float DefaultMaxWalkSpeed;

	// Wall run
	bool bWasWallRunIsRight;
	void TryWallRun();
	void PhysWallRun(float DeltaTime, int32 Iterations);

	float GetCapsuleRadius() const;
	float GetCapsuleHalfHeight() const;
	
};
