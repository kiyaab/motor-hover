// Copyright Epic Games, Inc. All Rights Reserved.

#include "HoverBikeStabilization.h"
#include "Core/HoverBikeMath.h"

UHoverBikeStabilization::UHoverBikeStabilization()
{
}

void UHoverBikeStabilization::Initialize(const UHoverBikeTuningData* TuningData)
{
	CachedTuningData = TuningData;
	CurrentLeanAngleDeg = 0.0f;
	TargetLeanAngleDeg = 0.0f;
	FilteredLateralAcceleration = 0.0f;
}

FVector UHoverBikeStabilization::CalculateSurfaceAlignmentTorque(
	const FTransform& VehicleTransform,
	const FVector& DesiredSurfaceNormal,
	const FVector& AngularVelocity,
	bool bGrounded,
	float AirborneTime) const
{
	const UHoverBikeTuningData* Tuning = CachedTuningData.Get();
	const float BaseStrength = (Tuning) ? Tuning->AlignmentStrength : 800000.0f;
	const float BaseDamping = (Tuning) ? Tuning->AlignmentDamping : 120000.0f;

	// Airborne attenuation: smooth decay over grace time
	float Authority = 1.0f;
	if (!bGrounded)
	{
		const float GraceTime = (Tuning) ? Tuning->ContactGraceTime : 0.2f;
		const float Alpha = FMath::Clamp((AirborneTime - GraceTime) / 1.0f, 0.0f, 1.0f);
		Authority = FMath::Lerp(0.5f, 0.15f, Alpha); // Preserve 15-50% subtle orientation assistance
	}

	const FVector CurrentUp = VehicleTransform.GetUnitAxis(EAxis::Z);
	const FVector DesiredUp = HoverBikeMath::SafeNormalize(DesiredSurfaceNormal, FVector::UpVector);

	return HoverBikeMath::CalculatePDAlignmentTorque(
		CurrentUp,
		DesiredUp,
		AngularVelocity,
		BaseStrength * Authority,
		BaseDamping * Authority,
		1500000.0f
	);
}

FVector UHoverBikeStabilization::CalculateGyroscopicTorque(
	const FTransform& VehicleTransform,
	const FVector& AngularVelocity,
	float SteeringInput) const
{
	const UHoverBikeTuningData* Tuning = CachedTuningData.Get();
	const float PitchKd = (Tuning) ? Tuning->GyroPitchDamping : 70000.0f;
	const float YawKd = (Tuning) ? Tuning->GyroYawDamping : 80000.0f;
	const float RollKd = (Tuning) ? Tuning->GyroRollDamping : 90000.0f;

	const FVector Forward = VehicleTransform.GetUnitAxis(EAxis::X);
	const FVector Right = VehicleTransform.GetUnitAxis(EAxis::Y);
	const FVector Up = VehicleTransform.GetUnitAxis(EAxis::Z);

	const float RollRate = FVector::DotProduct(AngularVelocity, Forward);
	const float PitchRate = FVector::DotProduct(AngularVelocity, Right);
	const float YawRate = FVector::DotProduct(AngularVelocity, Up);

	// Reduce yaw damping during intentional steering input so gyro doesn't fight player
	const float ActiveYawDamping = YawKd * (1.0f - 0.7f * FMath::Abs(SteeringInput));

	const FVector RollTorque = -Forward * (RollRate * RollKd);
	const FVector PitchTorque = -Right * (PitchRate * PitchKd);
	const FVector YawTorque = -Up * (YawRate * ActiveYawDamping);

	return RollTorque + PitchTorque + YawTorque;
}

void UHoverBikeStabilization::CalculateSteeringAndLeanTorque(
	const FTransform& VehicleTransform,
	const FVector& LinearVelocity,
	const FVector& AngularVelocity,
	const FVector& Acceleration,
	float EffectiveGravity,
	float SteeringInput,
	float DeltaTime,
	FVector& OutSteeringTorque,
	FVector& OutLeanTorque)
{
	OutSteeringTorque = FVector::ZeroVector;
	OutLeanTorque = FVector::ZeroVector;

	const UHoverBikeTuningData* Tuning = CachedTuningData.Get();
	const float ForwardSpeed = FVector::DotProduct(LinearVelocity, VehicleTransform.GetUnitAxis(EAxis::X));
	const float ForwardSpeedKmh = (ForwardSpeed * 0.036f);

	const float CounterSteerSpeedKmh = (Tuning) ? Tuning->CounterSteerFullSpeedKmh : 250.0f;
	const float SpeedAlpha = FMath::Clamp(ForwardSpeedKmh / CounterSteerSpeedKmh, 0.0f, 1.0f);

	const FVector Up = VehicleTransform.GetUnitAxis(EAxis::Z);
	const FVector Right = VehicleTransform.GetUnitAxis(EAxis::Y);
	const FVector Forward = VehicleTransform.GetUnitAxis(EAxis::X);

	// 1. Direct Steering Yaw Torque
	// At low speeds direct yaw dominates. At high speeds it tapers down in favor of lean-based turning
	const float BaseSteerTorque = (Tuning) ? Tuning->SteeringTorque : 450000.0f;
	const float DirectSteerMultiplier = FMath::Lerp(1.0f, 0.45f, SpeedAlpha);
	const float YawTorqueMagnitude = SteeringInput * BaseSteerTorque * DirectSteerMultiplier;
	OutSteeringTorque = Up * YawTorqueMagnitude;

	// 2. Dynamic Motorcycle Lean
	// Calculate lateral acceleration in bike's local coordinate frame
	const float InstantLateralAccel = FVector::DotProduct(Acceleration, Right);
	FilteredLateralAcceleration = FMath::FInterpTo(FilteredLateralAcceleration, InstantLateralAccel, DeltaTime, 12.0f);

	const float MaxLeanRad = FMath::DegreesToRadians((Tuning) ? Tuning->MaxLeanAngleDeg : 55.0f);

	// Theoretical lean angle from lateral acceleration: theta = atan(a_lat / g)
	// Also incorporate player steering input command to initiate prompt roll response
	const float DynamicLeanRad = HoverBikeMath::CalculateTargetLeanAngle(FilteredLateralAcceleration, EffectiveGravity, MaxLeanRad);
	const float CommandLeanRad = SteeringInput * MaxLeanRad * SpeedAlpha;

	// Blend measured acceleration lean with prompt steering command
	const float TargetRollRad = FMath::Clamp(DynamicLeanRad * 0.4f + CommandLeanRad * 0.6f, -MaxLeanRad, MaxLeanRad);
	TargetLeanAngleDeg = FMath::RadiansToDegrees(TargetRollRad);

	// Current roll relative to surface normal / local horizon
	const float CurrentRollRate = FVector::DotProduct(AngularVelocity, Forward);

	// Measure current roll angle relative to upright
	const FVector HorizonRight = FVector::CrossProduct(Forward, FVector::UpVector).GetSafeNormal();
	const float CurrentRollRad = FMath::Asin(FMath::Clamp(FVector::DotProduct(Right, FVector::UpVector), -1.0f, 1.0f));
	CurrentLeanAngleDeg = FMath::RadiansToDegrees(CurrentRollRad);

	const float RollAngleError = TargetRollRad - CurrentRollRad;

	// PD torque controller for dynamic roll lean
	const float KpRoll = (Tuning) ? Tuning->LeanTorque : 400000.0f;
	const float KdRoll = (Tuning) ? Tuning->LeanDamping : 90000.0f;

	const float RollTorqueMag = (RollAngleError * KpRoll) - (CurrentRollRate * KdRoll);
	OutLeanTorque = Forward * FMath::Clamp(RollTorqueMag, -800000.0f, 800000.0f);
}
