// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/HoverBikeTuningData.h"
#include "HoverBikeStabilization.generated.h"

/**
 * Handles PD orientation alignment to arbitrary surface normals (walls, loops, ceilings),
 * tri-axis gyroscopic angular stabilization, dynamic motorcycle leaning, and counter-steering.
 */
UCLASS()
class HOVERRACER_API UHoverBikeStabilization : public UObject
{
	GENERATED_BODY()

public:
	UHoverBikeStabilization();

	void Initialize(const UHoverBikeTuningData* TuningData);

	/**
	 * Computes orientation alignment torque to smoothly align the bike's local up vector
	 * with the dynamic surface normal without artificial rotational snapping.
	 */
	FVector CalculateSurfaceAlignmentTorque(
		const FTransform& VehicleTransform,
		const FVector& DesiredSurfaceNormal,
		const FVector& AngularVelocity,
		bool bGrounded,
		float AirborneTime) const;

	/**
	 * Calculates independent tri-axis gyroscopic stabilization torque.
	 * Damps unwanted oscillations while preserving intentional steering/leaning motion.
	 */
	FVector CalculateGyroscopicTorque(
		const FTransform& VehicleTransform,
		const FVector& AngularVelocity,
		float SteeringInput) const;

	/**
	 * Calculates dynamic motorcycle lean roll torque and speed-dependent steering torque.
	 * Includes realistic counter-steering at high speed.
	 *
	 * @param VehicleTransform Vehicle world transform
	 * @param LinearVelocity Rigid body velocity (cm/s)
	 * @param AngularVelocity Rigid body angular velocity (rad/s)
	 * @param Acceleration Rigid body acceleration (cm/s^2)
	 * @param EffectiveGravity Magnitude of effective downward gravity
	 * @param SteeringInput Player steering command [-1, 1]
	 * @param DeltaTime Substep delta time
	 * @param OutSteeringTorque Generated steering yaw torque
	 * @param OutLeanTorque Generated dynamic lean roll torque
	 */
	void CalculateSteeringAndLeanTorque(
		const FTransform& VehicleTransform,
		const FVector& LinearVelocity,
		const FVector& AngularVelocity,
		const FVector& Acceleration,
		float EffectiveGravity,
		float SteeringInput,
		float DeltaTime,
		FVector& OutSteeringTorque,
		FVector& OutLeanTorque);

	/** Telemetry readouts */
	float GetCurrentLeanAngleDeg() const { return CurrentLeanAngleDeg; }
	float GetTargetLeanAngleDeg() const { return TargetLeanAngleDeg; }

private:
	UPROPERTY()
	TWeakObjectPtr<const UHoverBikeTuningData> CachedTuningData;

	float CurrentLeanAngleDeg = 0.0f;
	float TargetLeanAngleDeg = 0.0f;
	float FilteredLateralAcceleration = 0.0f;
};
