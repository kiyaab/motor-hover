// Copyright Epic Games, Inc. All Rights Reserved.

#include "MotorcycleTireComponent.h"
#include "Engine/World.h"
#include "Core/MotorcycleMath.h"

UMotorcycleTireComponent::UMotorcycleTireComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UMotorcycleTireComponent::UpdateWheelPhysics(
	const FTransform& VehicleTransform,
	const FVector& LinearVelocity,
	const FVector& AngularVelocity,
	float DriveTorqueNm,
	float BrakePressureInput,
	float SteeringAngleDeg,
	float DynamicNormalLoadN,
	bool bEnableABS,
	float DeltaTime,
	UWorld* World,
	const FCollisionQueryParams& QueryParams,
	FVector& OutSuspensionForce,
	FVector& OutTireFrictionForce,
	FVector& OutContactLocation)
{
	OutSuspensionForce = FVector::ZeroVector;
	OutTireFrictionForce = FVector::ZeroVector;

	if (!World || DeltaTime <= 0.0f)
	{
		return;
	}

	const FVector HubWorldPos = GetComponentLocation();
	const FVector DownVector = -VehicleTransform.GetUnitAxis(EAxis::Z);
	OutContactLocation = HubWorldPos;

	// Trace from top of suspension stroke down to tire contact
	const float RestDistanceCm = SuspensionConfig.MaxTravel + SuspensionConfig.WheelRadius;
	const FVector TraceStart = HubWorldPos - (DownVector * (SuspensionConfig.MaxTravel * 0.5f));
	const FVector TraceEnd = HubWorldPos + (DownVector * (RestDistanceCm + 15.0f));

	FHitResult Hit;
	const bool bHit = World->SweepSingleByChannel(
		Hit,
		TraceStart,
		TraceEnd,
		FQuat::Identity,
		ECC_WorldStatic,
		FCollisionShape::MakeSphere(SuspensionConfig.WheelRadius * 0.5f),
		QueryParams
	);

	if (bHit && Hit.bBlockingHit && Hit.Distance > 0.0f)
	{
		WheelState.bOnGround = true;
		WheelState.ContactPoint = Hit.Location;
		WheelState.ContactNormal = Hit.ImpactNormal;
		OutContactLocation = Hit.Location;

		// 1. Suspension Compression (cm -> meters)
		const float ContactDist = FVector::DotProduct(Hit.Location - TraceStart, DownVector);
		const float FreeLength = RestDistanceCm + (SuspensionConfig.MaxTravel * 0.5f);
		const float CurrentCompressionCm = FMath::Clamp(FreeLength - ContactDist, 0.0f, SuspensionConfig.MaxTravel);
		WheelState.SuspensionCompression = CurrentCompressionCm;

		const float CompressionMeters = CurrentCompressionCm * 0.01f;
		const float CompVelocityMetersPerSec = ((CurrentCompressionCm - PreviousCompression) * 0.01f) / DeltaTime;
		PreviousCompression = CurrentCompressionCm;

		const float SuspForceMag = MotorcycleMath::CalculateSuspensionForce(
			CompressionMeters,
			CompVelocityMetersPerSec,
			SuspensionConfig.SpringStiffness,
			SuspensionConfig.DampingCoefficient
		);
		WheelState.SuspensionForce = SuspForceMag;
		OutSuspensionForce = WheelState.ContactNormal * SuspForceMag;

		// 2. Wheel Hub Point Velocity
		const FVector RelativePos = HubWorldPos - VehicleTransform.GetLocation();
		const FVector HubPointVelocity = LinearVelocity + FVector::CrossProduct(AngularVelocity, RelativePos);

		// Project forward and right relative to wheel steer angle
		FVector WheelForward = VehicleTransform.GetUnitAxis(EAxis::X);
		if (bIsFrontWheel && FMath::Abs(SteeringAngleDeg) > 0.01f)
		{
			WheelForward = WheelForward.RotateAngleAxis(SteeringAngleDeg, VehicleTransform.GetUnitAxis(EAxis::Z));
		}
		const FVector WheelRight = FVector::CrossProduct(WheelForward, VehicleTransform.GetUnitAxis(EAxis::Z)).GetSafeNormal();

		const float LongSpeedMetersPerSec = FVector::DotProduct(HubPointVelocity, WheelForward) * 0.01f;
		const float LatSpeedMetersPerSec = FVector::DotProduct(HubPointVelocity, WheelRight) * 0.01f;

		// 3. Wheel Spin & Longitudinal Slip
		const float WheelRadiusMeters = SuspensionConfig.WheelRadius * 0.01f;
		const float LinearSpeedWheel = WheelState.AngularVelocity * WheelRadiusMeters;

		// Slip ratio calculation: kappa = (r*omega - v) / max(|v|, |r*omega|)
		const float MaxDenominator = FMath::Max(0.2f, FMath::Max(FMath::Abs(LongSpeedMetersPerSec), FMath::Abs(LinearSpeedWheel)));
		float SlipRatio = (LinearSpeedWheel - LongSpeedMetersPerSec) / MaxDenominator;
		SlipRatio = FMath::Clamp(SlipRatio, -1.0f, 1.0f);
		WheelState.SlipRatio = SlipRatio;

		// 4. Brakes & Drive Torque
		float EffectiveBrakePressure = BrakePressureInput;
		if (bEnableABS && SlipRatio < -0.18f)
		{
			// ABS line pressure pulsing
			EffectiveBrakePressure *= 0.3f;
		}

		const float MaxBrakeTorque = bIsFrontWheel ? 650.0f : 250.0f; // Front dual disc produces ~70% stopping force
		const float BrakeTorque = EffectiveBrakePressure * MaxBrakeTorque;

		// Torque balance on wheel: I * d_omega/dt = DriveTorque - BrakeTorque - FrictionTorque
		const float WheelInertia = 0.65f; // kg*m^2
		float NetTorqueOnWheel = DriveTorqueNm;
		if (WheelState.AngularVelocity > 0.01f)
		{
			NetTorqueOnWheel -= BrakeTorque;
		}
		else if (WheelState.AngularVelocity < -0.01f)
		{
			NetTorqueOnWheel += BrakeTorque;
		}

		// Update angular velocity
		WheelState.AngularVelocity += (NetTorqueOnWheel / WheelInertia) * DeltaTime;
		if (FMath::Abs(LongSpeedMetersPerSec) < 0.1f && EffectiveBrakePressure > 0.5f)
		{
			WheelState.AngularVelocity = 0.0f;
		}

		// 5. Pacejka Tire Friction Forces
		const float EffectiveNormalForce = FMath::Max(100.0f, SuspForceMag + DynamicNormalLoadN);
		const float LongFrictionForce = MotorcycleMath::CalculatePacejkaFriction(SlipRatio, EffectiveNormalForce, ERoadSurfaceType::DryAsphalt);
		WheelState.LongitudinalForce = LongFrictionForce;

		// Lateral slip angle alpha = atan(v_y / |v_x|)
		const float SlipAngleRad = FMath::Atan2(LatSpeedMetersPerSec, FMath::Max(0.5f, FMath::Abs(LongSpeedMetersPerSec)));
		WheelState.SlipAngleDeg = FMath::RadiansToDegrees(SlipAngleRad);

		const float LatFrictionForce = -MotorcycleMath::CalculatePacejkaFriction(SlipAngleRad, EffectiveNormalForce, ERoadSurfaceType::DryAsphalt);
		WheelState.LateralForce = LatFrictionForce;

		// Combine friction forces in tire contact coordinate frame
		OutTireFrictionForce = (WheelForward * LongFrictionForce) + (WheelRight * LatFrictionForce);
	}
	else
	{
		WheelState.bOnGround = false;
		WheelState.SuspensionCompression = 0.0f;
		WheelState.SuspensionForce = 0.0f;
		WheelState.SlipRatio = 0.0f;
		WheelState.SlipAngleDeg = 0.0f;
		WheelState.LongitudinalForce = 0.0f;
		WheelState.LateralForce = 0.0f;
		PreviousCompression = 0.0f;
	}
}
