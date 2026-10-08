// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/MotorcycleTypes.h"
#include "MotorcyclePhysicsComponent.generated.h"

class UMotorcycleTireComponent;
class UMotorcycleEngineComponent;
class UPrimitiveComponent;

/**
 * 2-Wheel Chaos Physics Solver for realistic superbike dynamics.
 * Manages front/rear suspension, Pacejka tire friction, dynamic weight transfer,
 * wheelies, stoppies, motorcycle leaning, and crash mechanics.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HOVERRACER_API UMotorcyclePhysicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMotorcyclePhysicsComponent();

	virtual void InitializeComponent() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Executes 2-wheel physics step */
	void SimulateMotorcyclePhysics(float DeltaTime, UPrimitiveComponent* ChassisMesh);

	// Controls / Inputs
	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Control")
	void SetThrottleInput(float Value) { ThrottleInput = FMath::Clamp(Value, 0.0f, 1.0f); }

	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Control")
	void SetFrontBrakeInput(float Value) { FrontBrakeInput = FMath::Clamp(Value, 0.0f, 1.0f); }

	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Control")
	void SetRearBrakeInput(float Value) { RearBrakeInput = FMath::Clamp(Value, 0.0f, 1.0f); }

	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Control")
	void SetSteeringInput(float Value) { SteeringInput = FMath::Clamp(Value, -1.0f, 1.0f); }

	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Control")
	void SetRiderLeanInput(float Value) { RiderLeanInput = FMath::Clamp(Value, -1.0f, 1.0f); }

	// Telemetry
	UFUNCTION(BlueprintPure, Category = "Motorcycle|Telemetry")
	float GetSpeedKmh() const { return CurrentSpeedKmh; }

	UFUNCTION(BlueprintPure, Category = "Motorcycle|Telemetry")
	float GetCurrentLeanAngleDeg() const { return CurrentLeanAngleDeg; }

	UFUNCTION(BlueprintPure, Category = "Motorcycle|Telemetry")
	float GetTargetLeanAngleDeg() const { return TargetLeanAngleDeg; }

	UFUNCTION(BlueprintPure, Category = "Motorcycle|Telemetry")
	bool IsWheelie() const { return bIsWheelie; }

	UFUNCTION(BlueprintPure, Category = "Motorcycle|Telemetry")
	bool IsStoppie() const { return bIsStoppie; }

	UFUNCTION(BlueprintPure, Category = "Motorcycle|Telemetry")
	EMotorcycleCrashType GetCrashState() const { return CurrentCrashState; }

	UPROPERTY(BlueprintAssignable, Category = "Motorcycle|Events")
	FOnMotorcycleCrashSignature OnMotorcycleCrash;

	UPROPERTY(BlueprintAssignable, Category = "Motorcycle|Events")
	FOnWheelieStartedSignature OnWheelieStarted;

	UPROPERTY(BlueprintAssignable, Category = "Motorcycle|Events")
	FOnStoppieStartedSignature OnStoppieStarted;

	// Physical constants (1000cc Superbike)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Dimensions")
	float BikeMassKg = 195.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Dimensions")
	float RiderMassKg = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Dimensions")
	float WheelbaseCm = 144.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Dimensions")
	float CenterOfMassHeightCm = 56.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Dynamics")
	float MaxSteeringAngleDeg = 24.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Dynamics")
	float MaxLeanAngleDeg = 58.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Motorcycle|Dynamics")
	bool bEnableABS = true;

private:
	UPROPERTY()
	TObjectPtr<UMotorcycleTireComponent> FrontWheel;

	UPROPERTY()
	TObjectPtr<UMotorcycleTireComponent> RearWheel;

	UPROPERTY()
	TObjectPtr<UMotorcycleEngineComponent> EngineComponent;

	// Runtime inputs
	float ThrottleInput = 0.0f;
	float FrontBrakeInput = 0.0f;
	float RearBrakeInput = 0.0f;
	float SteeringInput = 0.0f;
	float RiderLeanInput = 0.0f;

	// Dynamics
	float CurrentSpeedKmh = 0.0f;
	float CurrentLeanAngleDeg = 0.0f;
	float TargetLeanAngleDeg = 0.0f;
	FVector PreviousVelocity = FVector::ZeroVector;
	FVector CalculatedAcceleration = FVector::ZeroVector;

	bool bIsWheelie = false;
	bool bIsStoppie = false;
	EMotorcycleCrashType CurrentCrashState = EMotorcycleCrashType::None;

	void CheckCrashConditions(float LateralForceDemand, float AvailableGrip, float LeanAngleRad);
};
