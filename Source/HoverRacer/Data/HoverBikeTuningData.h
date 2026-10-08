// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Core/HoverBikeTypes.h"
#include "HoverBikeTuningData.generated.h"

/**
 * Data-driven tuning asset containing all vehicle physical, aerodynamic,
 * suspension, propulsion, and stabilization parameters.
 * Defaults are configured to the Baseline Heavy Racing Bike specification.
 */
UCLASS(BlueprintType)
class HOVERRACER_API UHoverBikeTuningData : public UDataAsset
{
	GENERATED_BODY()

public:
	UHoverBikeTuningData();

	// ==========================================
	// Vehicle Physical Baseline
	// ==========================================

	/** Rigid body vehicle mass in kg */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (ClampMin = "50.0", ClampMax = "5000.0"))
	float VehicleMass = 350.0f;

	/** Maximum rated racing speed in km/h */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Vehicle", meta = (ClampMin = "50.0", ClampMax = "1200.0"))
	float MaxSpeedKmh = 650.0f;

	// ==========================================
	// Suspension & Repulsors
	// ==========================================

	/** Default equilibrium hover height (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float HoverHeight = 150.0f;

	/** Configurations for the 4 repulsor pads (FL, FR, RL, RR) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	TArray<FRepulsorPadConfig> RepulsorConfigs;

	/** Global multiplier for repulsor spring stiffness */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float GlobalSpringStiffnessMultiplier = 1.0f;

	/** Global multiplier for repulsor damping */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float GlobalDampingMultiplier = 1.0f;

	/** Repulsor grip energy maximum capacity */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float MaxGripEnergy = 100.0f;

	/** Grip energy consumption rate per second at maximum lateral slip */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float GripConsumptionRate = 35.0f;

	/** Grip energy passive recharge rate per second */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float GripRecoveryRate = 25.0f;

	// ==========================================
	// Surface Alignment & Stabilization
	// ==========================================

	/** Proportional alignment torque gain Kp for dynamic surface normal */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stabilization")
	float AlignmentStrength = 800000.0f;

	/** Derivative damping gain Kd for surface alignment */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stabilization")
	float AlignmentDamping = 120000.0f;

	/** Smoothing rate for surface normal interpolation (1/s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stabilization")
	float SurfaceNormalSmoothingRate = 18.0f;

	/** Contact grace duration (seconds) before airborne alignment attenuation starts */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stabilization")
	float ContactGraceTime = 0.2f;

	/** Gyro pitch angular damping torque */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stabilization")
	float GyroPitchDamping = 70000.0f;

	/** Gyro yaw angular damping torque */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stabilization")
	float GyroYawDamping = 80000.0f;

	/** Gyro roll angular damping torque */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stabilization")
	float GyroRollDamping = 90000.0f;

	// ==========================================
	// Propulsion & Boost
	// ==========================================

	/** Maximum forward thrust force in Newtons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Propulsion")
	float MaxForwardThrust = 180000.0f;

	/** Maximum reverse thrust force in Newtons */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Propulsion")
	float MaxReverseThrust = 50000.0f;

	/** Thrust multiplier when boost is engaged */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Propulsion")
	float BoostMultiplier = 1.65f;

	/** Maximum boost energy reservoir */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Propulsion")
	float MaxBoostEnergy = 100.0f;

	/** Boost energy consumption per second while boosting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Propulsion")
	float BoostConsumptionRate = 30.0f;

	/** Boost energy recharge rate per second while not boosting */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Propulsion")
	float BoostRechargeRate = 15.0f;

	// ==========================================
	// Steering & Dynamic Lean
	// ==========================================

	/** Maximum steering yaw torque (N*cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float SteeringTorque = 450000.0f;

	/** Speed at which counter-steering and lean fully replace direct yaw (km/h) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float CounterSteerFullSpeedKmh = 250.0f;

	/** Maximum roll lean angle in degrees */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering", meta = (ClampMin = "10.0", ClampMax = "75.0"))
	float MaxLeanAngleDeg = 55.0f;

	/** Roll torque gain to achieve target lean angle */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float LeanTorque = 400000.0f;

	/** Damping against rapid roll oscillation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Steering")
	float LeanDamping = 90000.0f;

	// ==========================================
	// Lateral Grip & Drift
	// ==========================================

	/** Maximum lateral magnetic/repulsor traction force (N) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traction")
	float MaxLateralGrip = 180000.0f;

	/** Fraction of lateral grip available during full drift (0.25 = 25%) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traction", meta = (ClampMin = "0.05", ClampMax = "0.8"))
	float DriftGripRatio = 0.25f;

	/** Lateral force demand ratio where drift starts initiating */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traction")
	float DriftStartThreshold = 0.85f;

	/** Lateral force demand ratio where vehicle is in 100% full drift */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traction")
	float DriftFullThreshold = 1.35f;

	// ==========================================
	// Aerodynamics & Air Brake
	// ==========================================

	/** Standard atmospheric air density (kg/m^3) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aerodynamics")
	float AirDensity = 1.225f;

	/** Baseline aerodynamic drag coefficient Cd */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aerodynamics")
	float DragCoefficient = 0.65f;

	/** Frontal projected cross-sectional area (m^2) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aerodynamics")
	float FrontalArea = 1.1f;

	/** Drag coefficient increase when air brake is fully deployed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aerodynamics")
	float AirBrakeCdIncrease = 1.8f;

	/** Reference downforce at rated speed (N) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aerodynamics")
	float BaseDownforce = 150000.0f;

	/** Maximum allowable aerodynamic downforce (N) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aerodynamics")
	float MaxDownforce = 150000.0f;

	// ==========================================
	// Recovery & Crash Mechanics
	// ==========================================

	/** Angle threshold in degrees from surface normal to trigger inversion detection */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
	float InversionAngleThresholdDeg = 95.0f;

	/** Time vehicle must be inverted or upside-down before recovery is recommended (s) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
	float InversionTimeout = 1.5f;

	/** Upright correction torque applied during soft recovery */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
	float SoftRecoveryTorque = 600000.0f;

	/** Repulsor range multiplier during assisted recovery */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
	float AssistedRecoveryRangeMultiplier = 1.8f;

	/** Impact energy threshold (N*m) to trigger crash state */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Recovery")
	float CrashImpactThreshold = 500000.0f;

	// ==========================================
	// Camera Dynamics
	// ==========================================

	/** Camera base field of view at low speed (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float BaseFOV = 90.0f;

	/** Camera field of view at maximum racing speed (degrees) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float MaxSpeedFOV = 115.0f;

	/** Default camera boom spring arm length (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraBoomLength = 480.0f;

	/** Camera boom vertical offset (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraBoomHeight = 140.0f;

	/** Speed of camera position lag interpolation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float CameraLagSpeed = 12.0f;
};
