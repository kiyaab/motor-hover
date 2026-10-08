// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Core/HoverBikeTypes.h"
#include "HoverBike.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UHoverBikePhysicsComponent;
class UHoverBikeInputComponent;
class UHoverBikeDebugComponent;
class UHoverBikeInputConfig;

/**
 * High-speed anti-gravity hoverbike vehicle pawn.
 * Driven strictly through genuine Chaos rigid body physics.
 */
UCLASS()
class HOVERRACER_API AHoverBike : public APawn
{
	GENERATED_BODY()

public:
	AHoverBike();

	virtual void PostInitializeComponents() override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;

	// Component Getters
	UFUNCTION(BlueprintPure, Category = "HoverBike|Components")
	UStaticMeshComponent* GetBikeMesh() const { return BikeMesh; }

	UFUNCTION(BlueprintPure, Category = "HoverBike|Components")
	UHoverBikePhysicsComponent* GetPhysicsComponent() const { return PhysicsComponent; }

	UFUNCTION(BlueprintPure, Category = "HoverBike|Components")
	UHoverBikeInputComponent* GetBikeInputComponent() const { return InputComponentInternal; }

	UFUNCTION(BlueprintPure, Category = "HoverBike|Components")
	UCameraComponent* GetCamera() const { return Camera; }

	// AI & Replay Input Control Interface
	UFUNCTION(BlueprintCallable, Category = "HoverBike|Control")
	void SetThrottleInput(float Value);

	UFUNCTION(BlueprintCallable, Category = "HoverBike|Control")
	void SetSteeringInput(float Value);

	UFUNCTION(BlueprintCallable, Category = "HoverBike|Control")
	void SetBrakeInput(float Value);

	UFUNCTION(BlueprintCallable, Category = "HoverBike|Control")
	void SetDriftInput(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "HoverBike|Control")
	void SetBoostInput(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "HoverBike|Control")
	void SetAirBrakeInput(float Value);

	UFUNCTION(BlueprintCallable, Category = "HoverBike|Control")
	void TriggerRecovery();

	// Procedural Audio & Visual Hook Properties
	UFUNCTION(BlueprintPure, Category = "HoverBike|Telemetry")
	float GetSpeedNormalized() const;

	UFUNCTION(BlueprintPure, Category = "HoverBike|Telemetry")
	float GetThrottleAmount() const;

	UFUNCTION(BlueprintPure, Category = "HoverBike|Telemetry")
	float GetBoostAmount() const;

	UFUNCTION(BlueprintPure, Category = "HoverBike|Telemetry")
	float GetDriftAmount() const;

	UFUNCTION(BlueprintPure, Category = "HoverBike|Telemetry")
	float GetRepulsorLoad() const;

	UFUNCTION(BlueprintPure, Category = "HoverBike|Telemetry")
	float GetAirborneAmount() const;

	// Gameplay Events
	UPROPERTY(BlueprintAssignable, Category = "HoverBike|Events")
	FOnRepulsorContactSignature OnRepulsorContact;

	UPROPERTY(BlueprintAssignable, Category = "HoverBike|Events")
	FOnRepulsorLostSignature OnRepulsorLost;

	UPROPERTY(BlueprintAssignable, Category = "HoverBike|Events")
	FOnDriftStartedSignature OnDriftStarted;

	UPROPERTY(BlueprintAssignable, Category = "HoverBike|Events")
	FOnDriftEndedSignature OnDriftEnded;

	UPROPERTY(BlueprintAssignable, Category = "HoverBike|Events")
	FOnBoostStartedSignature OnBoostStarted;

	UPROPERTY(BlueprintAssignable, Category = "HoverBike|Events")
	FOnBoostEndedSignature OnBoostEnded;

	UPROPERTY(BlueprintAssignable, Category = "HoverBike|Events")
	FOnAirborneSignature OnAirborne;

	UPROPERTY(BlueprintAssignable, Category = "HoverBike|Events")
	FOnLandingSignature OnLanding;

	UPROPERTY(BlueprintAssignable, Category = "HoverBike|Events")
	FOnCrashSignature OnCrash;

	/** Enhanced Input configuration data asset */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "HoverBike|Input")
	TObjectPtr<UHoverBikeInputConfig> InputConfig;

protected:
	/** Chaos rigid-body static mesh root component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HoverBike|Components")
	TObjectPtr<UStaticMeshComponent> BikeMesh;

	/** Central physics simulation component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HoverBike|Components")
	TObjectPtr<UHoverBikePhysicsComponent> PhysicsComponent;

	/** Input management component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HoverBike|Components")
	TObjectPtr<UHoverBikeInputComponent> InputComponentInternal;

	/** Camera spring arm boom */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HoverBike|Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	/** Player racing chase camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HoverBike|Components")
	TObjectPtr<UCameraComponent> Camera;

	/** Debug visualizer and telemetry HUD component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HoverBike|Components")
	TObjectPtr<UHoverBikeDebugComponent> DebugComponent;

private:
	void UpdateCameraDynamics(float DeltaSeconds);

	float CurrentCameraFOV = 90.0f;
	FRotator FilteredCameraRotation = FRotator::ZeroRotator;
};
