// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MotorcycleTypes.h"

/**
 * Pure mathematical formulas and physical equations for real 2-wheeled motorcycle simulation.
 */
namespace MotorcycleMath
{
	// ==========================================
	// Suspension Spring-Damper (F = -k*x - c*v)
	// ==========================================

	FORCEINLINE float CalculateSuspensionForce(
		float CompressionMeters,
		float CompressionVelocityMetersPerSec,
		float SpringStiffness,
		float DampingCoefficient,
		float MaxForce = 15000.0f)
	{
		const float SpringForce = SpringStiffness * CompressionMeters;
		const float DamperForce = DampingCoefficient * CompressionVelocityMetersPerSec;
		const float TotalForce = SpringForce + DamperForce;
		return FMath::Clamp(TotalForce, 0.0f, MaxForce);
	}

	// ==========================================
	// Dynamic Longitudinal Weight Transfer
	// ==========================================

	/**
	 * Computes dynamic weight shift between front and rear wheels during acceleration and braking.
	 * DeltaFz = Mass * AccelX * (CoM_Height / Wheelbase)
	 *
	 * @param TotalMass Total mass in kg (bike + rider)
	 * @param LongitudinalAccelMetersPerSec2 Forward acceleration in m/s^2 (positive = acceleration, negative = braking)
	 * @param CoMHeightMeters Height of center of mass above road (e.g. 0.55m)
	 * @param WheelbaseMeters Distance between front and rear axle centers (e.g. 1.44m)
	 * @return Load shifted onto rear axle in Newtons (negative shifts load onto front axle)
	 */
	FORCEINLINE float CalculateWeightTransfer(
		float TotalMass,
		float LongitudinalAccelMetersPerSec2,
		float CoMHeightMeters,
		float WheelbaseMeters)
	{
		if (WheelbaseMeters <= 0.1f)
		{
			return 0.0f;
		}
		return TotalMass * LongitudinalAccelMetersPerSec2 * (CoMHeightMeters / WheelbaseMeters);
	}

	// ==========================================
	// Pacejka Magic Formula Tire Friction Model
	// ==========================================

	/**
	 * Simplified Pacejka Magic Formula for longitudinal and lateral tire traction:
	 * F = D * sin(C * atan(B * slip - E * (B * slip - atan(B * slip))))
	 *
	 * @param SlipValue Longitudinal slip ratio (-1 to 1) or lateral slip angle (radians)
	 * @param NormalForce Normal downward load Fz in Newtons
	 * @param SurfaceType Road physical surface type
	 * @return Friction force in Newtons
	 */
	FORCEINLINE float CalculatePacejkaFriction(
		float SlipValue,
		float NormalForce,
		ERoadSurfaceType SurfaceType)
	{
		if (NormalForce <= 0.0f)
		{
			return 0.0f;
		}

		float MuPeak = 1.05f; // Dry asphalt standard superbike tire
		switch (SurfaceType)
		{
		case ERoadSurfaceType::DryAsphalt: MuPeak = 1.08f; break;
		case ERoadSurfaceType::WetAsphalt: MuPeak = 0.72f; break;
		case ERoadSurfaceType::Gravel:     MuPeak = 0.45f; break;
		case ERoadSurfaceType::Dirt:       MuPeak = 0.42f; break;
		case ERoadSurfaceType::Grass:      MuPeak = 0.32f; break;
		case ERoadSurfaceType::Concrete:   MuPeak = 0.95f; break;
		}

		const float B = 10.0f;  // Stiffness factor
		const float C = 1.65f;  // Shape factor
		const float D = NormalForce * MuPeak; // Peak force
		const float E = -0.4f;  // Curvature factor

		const float Bx = B * SlipValue;
		const float Curve = Bx - E * (Bx - FMath::Atan(Bx));
		return D * FMath::Sin(C * FMath::Atan(Curve));
	}

	// ==========================================
	// Dynamic Motorcycle Leaning: lean = atan(v^2 / (r*g))
	// ==========================================

	/**
	 * Computes dynamic theoretical motorcycle lean angle from velocity and lateral acceleration:
	 * theta = atan(v^2 / (r * g)) = atan(a_lateral / g)
	 *
	 * @param LateralAccelMetersPerSec2 Centripetal acceleration in m/s^2
	 * @param GravityMetersPerSec2 Downward gravity (standard 9.81 m/s^2)
	 * @param MaxLeanAngleDegrees Maximum superbike lean limit (standard 58 degrees)
	 * @return Lean angle in radians (positive leans right)
	 */
	FORCEINLINE float CalculateMotorcycleLean(
		float LateralAccelMetersPerSec2,
		float GravityMetersPerSec2 = 9.81f,
		float MaxLeanAngleDegrees = 58.0f)
	{
		const float SafeG = FMath::Max(9.0f, GravityMetersPerSec2);
		const float IdealLean = FMath::Atan2(LateralAccelMetersPerSec2, SafeG);
		const float MaxLeanRad = FMath::DegreesToRadians(MaxLeanAngleDegrees);
		return FMath::Clamp(IdealLean, -MaxLeanRad, MaxLeanRad);
	}

	// ==========================================
	// Engine Torque & Power Curve (1000cc Superbike)
	// ==========================================

	/**
	 * Evaluates 4-cylinder superbike engine torque in Nm across RPM range (1,200 to 15,000 RPM).
	 * Peak torque ~ 118 Nm @ 11,000 RPM (~ 205 HP @ 13,500 RPM).
	 *
	 * @param RPM Current engine revolutions per minute
	 * @return Output crankshaft torque in Newton-meters (Nm)
	 */
	FORCEINLINE float EvaluateEngineTorque(float RPM)
	{
		if (RPM < 1200.0f)
		{
			return 40.0f; // Idle torque
		}
		if (RPM > 14800.0f)
		{
			// Soft rev limiter
			return FMath::Max(0.0f, 110.0f - (RPM - 14800.0f) * 0.25f);
		}

		// Normalized curve: rise to peak at 11,000 RPM, broad plateau to 13,500 RPM
		const float X = FMath::Clamp((RPM - 1200.0f) / (11000.0f - 1200.0f), 0.0f, 1.4f);
		if (X <= 1.0f)
		{
			return FMath::Lerp(55.0f, 118.0f, FMath::Sin(X * 1.57079f));
		}
		else
		{
			// Past peak, torque tapers slightly as horsepower peaks
			const float Beyond = (X - 1.0f) / 0.4f;
			return FMath::Lerp(118.0f, 98.0f, Beyond * Beyond);
		}
	}
}
