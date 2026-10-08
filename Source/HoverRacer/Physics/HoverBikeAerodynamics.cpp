// Copyright Epic Games, Inc. All Rights Reserved.

#include "HoverBikeAerodynamics.h"
#include "Core/HoverBikeMath.h"

UHoverBikeAerodynamics::UHoverBikeAerodynamics()
{
}

void UHoverBikeAerodynamics::Initialize(const UHoverBikeTuningData* TuningData)
{
	CachedTuningData = TuningData;
}

FVector UHoverBikeAerodynamics::CalculateDragForce(const FVector& LinearVelocity, float AirBrakeInput) const
{
	const float Speed = LinearVelocity.Size();
	if (Speed < 5.0f)
	{
		LastDragMagnitude = 0.0f;
		return FVector::ZeroVector;
	}

	const UHoverBikeTuningData* Tuning = CachedTuningData.Get();
	const float AirDensity = (Tuning) ? Tuning->AirDensity : 1.225f;
	const float BaseCd = (Tuning) ? Tuning->DragCoefficient : 0.65f;
	const float Area = (Tuning) ? Tuning->FrontalArea : 1.1f;
	const float AirBrakeCdAdd = (Tuning) ? Tuning->AirBrakeCdIncrease : 1.8f;

	const float EffectiveCd = BaseCd + (FMath::Clamp(AirBrakeInput, 0.0f, 1.0f) * AirBrakeCdAdd);

	const float DragForceMag = HoverBikeMath::CalculateQuadraticDrag(Speed, AirDensity, EffectiveCd, Area);
	LastDragMagnitude = DragForceMag;

	const FVector VelocityDir = LinearVelocity / Speed;
	return -VelocityDir * DragForceMag;
}

FVector UHoverBikeAerodynamics::CalculateDownforce(
	const FVector& LinearVelocity,
	const FVector& SurfaceNormal,
	float AverageHoverHeight,
	float GroundedRatio) const
{
	if (GroundedRatio <= 0.05f)
	{
		LastDownforceMagnitude = 0.0f;
		return FVector::ZeroVector;
	}

	const UHoverBikeTuningData* Tuning = CachedTuningData.Get();
	const float Speed = LinearVelocity.Size();
	const float MaxSpeedCm = (Tuning) ? Tuning->MaxSpeedKmh * (100000.0f / 3600.0f) : 18000.0f;
	const float BaseDownforce = (Tuning) ? Tuning->BaseDownforce : 150000.0f;
	const float MaxHoverHeight = (Tuning) ? Tuning->HoverHeight * 2.0f : 300.0f;
	const float MaxDownforce = (Tuning) ? Tuning->MaxDownforce : 150000.0f;

	const float DownforceMag = HoverBikeMath::CalculateGroundEffectDownforce(
		Speed,
		BaseDownforce,
		MaxSpeedCm * 0.7f,
		AverageHoverHeight,
		MaxHoverHeight,
		MaxDownforce
	) * GroundedRatio;

	LastDownforceMagnitude = DownforceMag;

	// Downforce pushes toward the driving surface along negative surface normal
	return -SurfaceNormal * DownforceMag;
}

FVector UHoverBikeAerodynamics::CalculateAeroStabilizationTorque(
	const FVector& LinearVelocity,
	const FVector& AngularVelocity,
	const FTransform& VehicleTransform,
	float AirBrakeInput) const
{
	const float Speed = LinearVelocity.Size();
	if (Speed < 50.0f)
	{
		return FVector::ZeroVector;
	}

	// At high speeds, aerodynamic fin forces create natural yaw/pitch damping
	const float SpeedNormalized = FMath::Clamp(Speed / 18000.0f, 0.0f, 1.5f);
	const float DynamicPressureFactor = SpeedNormalized * SpeedNormalized;

	const FVector Forward = VehicleTransform.GetUnitAxis(EAxis::X);
	const FVector Right = VehicleTransform.GetUnitAxis(EAxis::Y);
	const FVector Up = VehicleTransform.GetUnitAxis(EAxis::Z);

	// Yaw damping from vertical stabilizer fin
	const float YawVelocity = FVector::DotProduct(AngularVelocity, Up);
	const float AeroYawTorque = -YawVelocity * DynamicPressureFactor * 40000.0f;

	// Pitch damping from horizontal canards/tailplane
	const float PitchVelocity = FVector::DotProduct(AngularVelocity, Right);
	const float AeroPitchTorque = -PitchVelocity * DynamicPressureFactor * 35000.0f;

	// Air brake flaps add extra stabilizing pitch-up and yaw resistance
	const float AirBrakeClamped = FMath::Clamp(AirBrakeInput, 0.0f, 1.0f);
	const float BrakePitchTorque = AirBrakeClamped * DynamicPressureFactor * 25000.0f;

	const FVector TotalAeroTorque = (Up * AeroYawTorque) + (Right * (AeroPitchTorque + BrakePitchTorque));
	return TotalAeroTorque;
}
