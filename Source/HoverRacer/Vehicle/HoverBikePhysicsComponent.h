// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/HoverBikeTypes.h"
#include "Data/HoverBikeTuningData.h"
#include "Physics/RepulsorSystem.h"
#include "Physics/HoverBikeAerodynamics.h"
#include "Physics/HoverBikeStabilization.h"
#include "Physics/HoverBikeDriftSystem.h"
#include "HoverBikePhysicsComponent.generated.h"

class UPrimitiveComponent;

/**
 * Orchestrates the full 14-step rigid body physics simulation pipeline,
 * state machine transitions, recovery mechanics, and crash detection.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HOVERRACER_API UHoverBikePhysicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHoverBikePhysicsComponent();

	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Executes the custom physics step on the Chaos rigid body */
	void SimulatePhysics(float DeltaTime, UPrimitiveComponent* TargetMesh);

	/** Recovery triggers */
	UFUNCTION(BlueprintCallable, Category = "HoverBike|Recovery")
	void TriggerRecovery();

	/** AI and external input query hooks */
	UFUNCTION(BlueprintPure, Category = "HoverBike|Physics")
	EHoverBikeState GetCurrentState() const { return CurrentState; }

	UFUNCTION(BlueprintPure, Category = "HoverBike|Physics")
	float GetCurrentSpeedKmh() const { return CurrentSpeedKmh; }

	UFUNCTION(BlueprintPure, Category = "HoverBike|Physics")
	float GetForwardSpeedKmh() const { return ForwardSpeedKmh; }

	UFUNCTION(BlueprintPure, Category = "HoverBike|Physics")
	float GetBoostEnergyPercent() const;

	UFUNCTION(BlueprintPure, Category = "HoverBike|Physics")
	float GetGripEnergyPercent() const;

	UFUNCTION(BlueprintPure, Category = "HoverBike|Physics")
	float GetCurrentDriftAlpha() const;

	UFUNCTION(BlueprintPure, Category = "HoverBike|Physics")
	float GetHoverHeight() const;

	UFUNCTION(BlueprintPure, Category = "HoverBike|Physics")
	int32 GetGroundedPadCount() const;

	UFUNCTION(BlueprintPure, Category = "HoverBike|Physics")
	FVector GetSmoothedSurfaceNormal() const;

	UFUNCTION(BlueprintPure, Category = "HoverBike|Physics")
	FHoverBikePhysicsState GetPhysicsStateSnapshot() const { return StateSnapshot; }

	/** Subsystem accessors */
	URepulsorSystem* GetRepulsorSystem() const { return RepulsorSystem; }
	UHoverBikeAerodynamics* GetAerodynamics() const { return Aerodynamics; }
	UHoverBikeStabilization* GetStabilization() const { return Stabilization; }
	UHoverBikeDriftSystem* GetDriftSystem() const { return DriftSystem; }

	/** Tuning data asset reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HoverBike|Tuning")
	TObjectPtr<UHoverBikeTuningData> TuningData;

	// Telemetry data for HUD/Debug
	float LastThrustApplied = 0.0f;
	float LastDownforceApplied = 0.0f;
	float LastDragApplied = 0.0f;
	FVector LastSteeringTorque = FVector::ZeroVector;
	FVector LastLeanTorque = FVector::ZeroVector;

private:
	// Subsystems
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HoverBike|Physics", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<URepulsorSystem> RepulsorSystem;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HoverBike|Physics", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHoverBikeAerodynamics> Aerodynamics;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HoverBike|Physics", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHoverBikeStabilization> Stabilization;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HoverBike|Physics", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UHoverBikeDriftSystem> DriftSystem;

	// Runtime state
	EHoverBikeState CurrentState = EHoverBikeState::Grounded;
	FHoverBikePhysicsState StateSnapshot;

	FVector PreviousLinearVelocity = FVector::ZeroVector;
	FVector CalculatedAcceleration = FVector::ZeroVector;

	float CurrentSpeedKmh = 0.0f;
	float ForwardSpeedKmh = 0.0f;
	float AirborneTime = 0.0f;
	float InversionTimer = 0.0f;
	float BoostEnergy = 100.0f;

	// Safe track checkpoint tracking
	FTransform LastSafeTransform;
	FVector LastSafeSurfaceNormal = FVector::UpVector;
	FVector LastSafeVelocity = FVector::ZeroVector;
	float TimeSinceLastSafe = 0.0f;

	// Recovery state
	bool bIsRecovering = false;
	float RecoveryTimer = 0.0f;

	// Internal simulation steps
	void UpdateStateMachine(bool bGrounded, int32 GroundedCount, const FVector& SurfaceNormal, const FTransform& VehicleTransform);
	void UpdateSafeTrackState(float DeltaTime, const FTransform& VehicleTransform, const FVector& Velocity, const FVector& SurfaceNormal, bool bGrounded);
	FVector QueryEffectiveGravity(const FVector& WorldLocation) const;
	void CheckCrashConditions(const FVector& LinearVelocity, const FVector& AngularVelocity, float DeltaTime, UPrimitiveComponent* TargetMesh);
};
