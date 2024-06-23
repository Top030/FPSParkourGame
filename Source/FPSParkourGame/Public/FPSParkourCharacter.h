// Copyright 2024 Mizunona. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "FPSParkourCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UCameraComponent;
class UFPSParkourCharacterComponent;

UCLASS()
class FPSPARKOURGAME_API AFPSParkourCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AFPSParkourCharacter(const FObjectInitializer& ObjectInitializer);
	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	FORCEINLINE UFPSParkourCharacterComponent* GetFPSParkourCharacterMovement() const{ return FPSParkourCharacterComponent; }

	FCollisionQueryParams GetIgnoreCharacterParams() const;

	UFUNCTION(BlueprintCallable, Category= "Movement")
	void CharacterMovement(const FInputActionValue& ActionValue);

	UFUNCTION(BlueprintCallable, Category= "Movement")
	void CharacterLook(const FInputActionValue& ActionValue);

	UFUNCTION(BlueprintCallable, Category= "Movement")
	void CharacterSprint(const FInputActionValue& ActionValue);

	UFUNCTION(BlueprintCallable, Category= "Movement")
	void CharacterCrouch(const FInputActionValue& ActionValue);

	UCameraComponent* GetCameraComponent() const;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Movement")
	UFPSParkourCharacterComponent* FPSParkourCharacterComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Input", meta= (AllowPrivateAccess= "true"))
	UInputMappingContext* DefaultInputMappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Input", meta= (AllowPrivateAccess= "true"))
	UInputAction* IA_Move;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Input", meta= (AllowPrivateAccess= "true"))
	UInputAction* IA_Look;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Input", meta= (AllowPrivateAccess= "true"))
	UInputAction* IA_Jump;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Input", meta= (AllowPrivateAccess= "true"))
	UInputAction* IA_Sprint;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category= "Input", meta= (AllowPrivateAccess= "true"))
	UInputAction* IA_Crouch;

private:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category= "Gameplay", meta= (AllowPrivateAccess= "true"))
	UCameraComponent* FPSCameraComponent;
	
};
