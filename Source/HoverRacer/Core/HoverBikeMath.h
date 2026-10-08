// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

/**
 * Pure mathematical utilities and physics formulas for anti-gravity hoverbike simulation.
 * Guaranteed numerically stable, protected against division-by-zero, NaNs, and infinities.
 */
namespace HoverBikeMath
{
	// ==========================================
	// Numerical Stability Helpers
	// ==========================================

	FORCEINLINE bool IsFiniteVector(const FVector& V)
	{
		return FMath::IsFinite(V.X) && FMath::IsFinite(V.Y) && FMath::IsFinite(V.Z);
	}

	FORCEINLINE bool IsFiniteQuat(const FQuat& Q)
	{
		return FMath::IsFinite(Q.X) && FMath::IsFinite(Q.Y) && FMath::IsFinite(Q.Z) && FMath::IsFinite(Q.W);
	}

	FORCEINLINE FVector SafeNormalize(const FVector& V, const FVector& Fallback = FVector::UpVector, float Tolerance = 1.e-6f)
	{
		if (!IsFiniteVector(V))
		{
			return Fallback;
		}

		const float SquareSum = V.SizeSquared();
		if (SquareSum > Tolerance * Tolerance)
		{
			const float Scale = FMath::InvSqrt(SquareSum);
			return V * Scale;
		}
		return Fallback;
	}

	FORCEINLINE float SafeDivide(float Numerator, float Denominator, float Fallback = 0.0f, float Tolerance = 1.e-6f)
	{
		if (!FMath::IsFinite(Numerator) || !FMath::IsFinite(Denominator))
		{
			return Fallback;
		}
		if (FMath::Abs(Denominator) > Tolerance)
		{
			return Numerator / Denominator;
		}
		return Fallback;
	}

	FORCEINLINE float SmoothStep(float Edge0, float Edge1, float X)
	{
		const float T = FMath::Clamp(SafeDivide(X - Edge0, Edge1 - Edge0, 0.0f), 0.0f, 1.0f);
		return T * T * (3.0f - 2.0f * T);
	}

	// ==========================================
	// Rigid Body Kinematics
	// ==========================================

	/**
	 * Calculates the instantaneous linear velocity of a specific point on the rigid body.
	 * v_point = v_com + omega x r
	 *
	 * @param LinearVelocity Center of mass linear velocity (cm/s)
	 * @param AngularVelocity Center of mass angular velocity (rad/s)
	 * @param RelativePosition Vector from center of mass to point (cm)
	 * @return Point velocity in world space (cm/s)
	 */
	FORCEINLINE FVector CalculatePointVelocity(
		const FVector& LinearVelocity,
		const FVector& AngularVelocity,
		const FVector& RelativePosition)
	{
		if (!IsFiniteVector(LinearVelocity) || !IsFiniteVector(AngularVelocity) || !IsFiniteVector(RelativePosition))
		{
			return FVector::ZeroVector;
		}
		return LinearVelocity + FVector::CrossProduct(AngularVelocity, RelativePosition);
	}

	// ==========================================
	// Repulsor Spring-Damper Formula
	// ==========================================

	/**
	 * Physically based damped spring formula:
	 * F = k * (RestLength - CurrentLength) - c * (v_point . n)
	 * Clamped to [0, MaxForce]
	 *
	 * @param RestLength Equilibrium hover height (cm)
	 * @param CurrentLength Measured distance along local down to surface (cm)
	 * @param SpringStiffness k (N/m or scaled UE spring rate)
	 * @param DampingCoefficient c (N*s/m or scaled UE damping rate)
	 * @param VelocityAlongNormal Speed of the repulsor point projected onto surface normal (cm/s)
	 * @param MaxForce Maximum allowed repulsor force (N)
	 * @return Applied force magnitude along surface normal (N)
	 */
	FORCEINLINE float CalculateDampedSpringForce(
		float RestLength,
		float CurrentLength,
		float SpringStiffness,
		float DampingCoefficient,
		float VelocityAlongNormal,
		float MaxForce)
	{
		const float Compression = RestLength - CurrentLength;
		// Spring force: pushes back against compression (positive when compressed)
		const float SpringForce = SpringStiffness * Compression;
		// Damper force: resists velocity into the surface (VelocityAlongNormal negative when approaching)
		// VelocityAlongNormal = v_point . n. If moving down toward surface, dot product is negative, so -c * v is positive
		const float DamperForce = DampingCoefficient * VelocityAlongNormal;

		const float TotalForce = SpringForce - DamperForce;
		return FMath::Clamp(TotalForce, 0.0f, MaxForce);
	}

	// ==========================================
	// Orientation & PD Alignment
	// ==========================================

	/**
	 * Computes torque required to align CurrentUp vector with DesiredUp vector using a PD controller.
	 * Torque = Kp * theta * axis - Kd * omega
	 *
	 * @param CurrentUp Normalized current unit Z axis of the vehicle
	 * @param DesiredUp Normalized desired surface normal
	 * @param AngularVelocity Current angular velocity in rad/s
	 * @param AlignmentStrength Proportional gain Kp
	 * @param AlignmentDamping Derivative gain Kd
	 * @param MaxTorque Torque ceiling for stability
	 * @return Torque vector to apply to rigid body (N*cm or UE torque units)
	 */
	FORCEINLINE FVector CalculatePDAlignmentTorque(
		const FVector& CurrentUp,
		const FVector& DesiredUp,
		const FVector& AngularVelocity,
		float AlignmentStrength,
		float AlignmentDamping,
		float MaxTorque)
	{
		const FVector CrossAxis = FVector::CrossProduct(CurrentUp, DesiredUp);
		const float SinAngle = CrossAxis.Size();
		const float DotAngle = FMath::Clamp(FVector::DotProduct(CurrentUp, DesiredUp), -1.0f, 1.0f);
		const float AngleError = FMath::Atan2(SinAngle, DotAngle);

		if (AngleError < 1.e-4f || SinAngle < 1.e-4f)
		{
			// Already aligned; damp any residual angular velocity perpendicular to up
			const FVector DampingTorque = -AngularVelocity * AlignmentDamping;
			return DampingTorque.GetClampedToMaxSize(MaxTorque);
		}

		const FVector RotationAxis = CrossAxis / SinAngle;
		const FVector ProportionalTorque = RotationAxis * (AngleError * AlignmentStrength);
		const FVector DerivativeTorque = -AngularVelocity * AlignmentDamping;

		const FVector TotalTorque = ProportionalTorque + DerivativeTorque;
		return TotalTorque.GetClampedToMaxSize(MaxTorque);
	}

	// ==========================================
	// Motorcycle Lean & Counter-Steer
	// ==========================================

	/**
	 * Computes dynamic theoretical motorcycle lean angle from lateral acceleration.
	 * theta = atan(a_lateral / g_effective)
	 *
	 * @param LateralAcceleration Acceleration along vehicle's right axis (cm/s^2)
	 * @param EffectiveGravity Magnitude of effective downward gravity (cm/s^2)
	 * @param MaxLeanAngleRadians Maximum allowed lean angle (rad)
	 * @return Target roll angle in radians (positive = roll right)
	 */
	FORCEINLINE float CalculateTargetLeanAngle(
		float LateralAcceleration,
		float EffectiveGravity,
		float MaxLeanAngleRadians)
	{
		const float SafeG = FMath::Max(FMath::Abs(EffectiveGravity), 100.0f);
		const float IdealLean = FMath::Atan2(LateralAcceleration, SafeG);
		return FMath::Clamp(IdealLean, -MaxLeanAngleRadians, MaxLeanAngleRadians);
	}

	// ==========================================
	// Aerodynamics: Quadratic Drag & Downforce
	// ==========================================

	/**
	 * Quadratic aerodynamic drag force:
	 * F_d = 0.5 * rho * v^2 * Cd * Area
	 *
	 * @param Speed Velocity magnitude in cm/s (converted to m/s internally)
	 * @param AirDensity Air density rho (kg/m^3, standard ~1.225)
	 * @param DragCoefficient Non-dimensional drag coefficient Cd
	 * @param FrontalArea Projected frontal area in m^2
	 * @return Drag force magnitude in Newtons
	 */
	FORCEINLINE float CalculateQuadraticDrag(
		float Speed,
		float AirDensity,
		float DragCoefficient,
		float FrontalArea)
	{
		const float SpeedMetersPerSecond = Speed * 0.01f;
		const float SpeedSquared = SpeedMetersPerSecond * SpeedMetersPerSecond;
		return 0.5f * AirDensity * SpeedSquared * DragCoefficient * FrontalArea;
	}

	/**
	 * Dynamic ground-effect downforce:
	 * Scales with speed squared and proximity to surface (1 - hover_height / max_height)
	 *
	 * @param Speed Velocity magnitude in cm/s
	 * @param BaseDownforce Reference downforce constant (N)
	 * @param ReferenceSpeed Nominal speed for full downforce (cm/s)
	 * @param AverageHoverHeight Current measured average hover height (cm)
	 * @param MaxHoverHeight Maximum height above which ground effect ceases (cm)
	 * @param MaxDownforce Hard clamp for downforce (N)
	 * @return Downforce magnitude in Newtons
	 */
	FORCEINLINE float CalculateGroundEffectDownforce(
		float Speed,
		float BaseDownforce,
		float ReferenceSpeed,
		float AverageHoverHeight,
		float MaxHoverHeight,
		float MaxDownforce)
	{
		if (ReferenceSpeed <= 1.0f || MaxHoverHeight <= 1.0f)
		{
			return 0.0f;
		}

		const float SpeedRatio = FMath::Clamp(Speed / ReferenceSpeed, 0.0f, 2.5f);
		const float SpeedFactor = SpeedRatio * SpeedRatio;

		const float ProximityRatio = FMath::Clamp(1.0f - (AverageHoverHeight / MaxHoverHeight), 0.0f, 1.0f);
		const float HeightFactor = ProximityRatio * ProximityRatio; // Non-linear ground effect boost

		const float Downforce = BaseDownforce * SpeedFactor * HeightFactor;
		return FMath::Clamp(Downforce, 0.0f, MaxDownforce);
	}

	// ==========================================
	// Engine Power Curve
	// ==========================================

	/**
	 * Realistic torque/power curve for hybrid futuristic electric thruster:
	 * Full instant torque at low speed, strong mid-range, tapering smoothly near top speed.
	 *
	 * @param NormalizedSpeed Current speed / MaxSpeed [0, 1]
	 * @return Multiplier in [0, 1]
	 */
	FORCEINLINE float EvaluateEnginePowerCurve(float NormalizedSpeed)
	{
		const float Alpha = FMath::Clamp(NormalizedSpeed, 0.0f, 1.5f);
		if (Alpha >= 1.0f)
		{
			// Beyond rated top speed, thrust drops exponentially to prevent run-away
			return FMath::Max(0.0f, 1.0f - (Alpha - 1.0f) * 4.0f);
		}
		// Cubic blend: flat torque table transitioning into constant power
		// 1.0 at 0, 0.95 at 0.3, 0.8 at 0.6, 0.4 at 0.9, 0.1 at 1.0
		const float T = 1.0f - Alpha;
		return FMath::Clamp(0.15f + 0.85f * (T * T * (3.0f - 2.0f * T)), 0.0f, 1.0f);
	}
}
