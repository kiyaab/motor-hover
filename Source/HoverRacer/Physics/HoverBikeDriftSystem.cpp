// Copyright Epic Games, Inc. All Rights Reserved.

#include "HoverBikeDriftSystem.h"
#include "Core/HoverBikeMath.h"

UHoverBikeDriftSystem::UHoverBikeDriftSystem()
{
}

void UHoverBikeDriftSystem::Initialize(const UHoverBikeTuningData* TuningData)
{
	CachedTuningData = TuningData;
	CurrentDriftAlpha = 0.0f;
	LastGripDemandRatio = 0.0f;
	bWasDrifting = false;
}

float UHoverBikeDriftSystem::CalculateLateralTractionForce(
	const FTransform& VehicleTransform,
	const FVector& LinearVelocity,
	float VehicleMass,
	float GroundedRatio,
	float GripEnergyPercent,
	bool bDriftInput,
	float DeltaTime,
	FVector& OutLateralForce)
{
	OutLateralForce = FVector::ZeroVector;

	if (GroundedRatio <= 0.05f || DeltaTime <= 0.0f)
	{
		CurrentDriftAlpha = 1.0f;
		LastGripDemandRatio = 2.0f;
		return CurrentDriftAlpha;
	}

	const UHoverBikeTuningData* Tuning = CachedTuningData.Get();
	const FVector Right = VehicleTransform.GetUnitAxis(EAxis::Y);

	// Instantaneous sideways slip speed (cm/s)
	const float LateralSpeed = FVector::DotProduct(LinearVelocity, Right);

	if (FMath::Abs(LateralSpeed) < 1.0f)
	{
		CurrentDriftAlpha = 0.0f;
		LastGripDemandRatio = 0.0f;
		return 0.0f;
	}

	// Calculate force required to completely cancel lateral drift this frame:
	// F = m * a = m * (dv / dt) (converted from cm/s to m/s for SI Newtons: 1 cm/s = 0.01 m/s)
	const float DesiredLateralAccelMeters = (LateralSpeed * 0.01f) / DeltaTime;
	const float RequiredLateralForce = VehicleMass * DesiredLateralAccelMeters;
	const float RequiredForceMagnitude = FMath::Abs(RequiredLateralForce);

	// Compute available magnetic repulsor grip capacity
	const float BaseGrip = (Tuning) ? Tuning->MaxLateralGrip : 180000.0f;
	const float EnergyFactor = FMath::Clamp(GripEnergyPercent * 0.01f, 0.25f, 1.0f);
	float AvailableGrip = BaseGrip * EnergyFactor * GroundedRatio;

	// Intentional drift input reduces grip threshold to induce power-sliding
	if (bDriftInput)
	{
		AvailableGrip *= 0.35f;
	}

	// Grip demand ratio
	const float DemandRatio = HoverBikeMath::SafeDivide(RequiredForceMagnitude, AvailableGrip, 2.0f);
	LastGripDemandRatio = DemandRatio;

	// Dynamic breakdown model using SmoothStep
	const float StartThresh = (Tuning) ? Tuning->DriftStartThreshold : 0.85f;
	const float FullThresh = (Tuning) ? Tuning->DriftFullThreshold : 1.35f;
	const float TargetDriftAlpha = HoverBikeMath::SmoothStep(StartThresh, FullThresh, DemandRatio);

	// Smooth drift transition to avoid sudden harsh snaps
	CurrentDriftAlpha = FMath::FInterpTo(CurrentDriftAlpha, TargetDriftAlpha, DeltaTime, 14.0f);

	// Blend effective grip based on drift alpha
	const float DriftGripRatio = (Tuning) ? Tuning->DriftGripRatio : 0.25f;
	const float EffectiveGripMultiplier = FMath::Lerp(1.0f, DriftGripRatio, CurrentDriftAlpha);
	const float MaxEffectiveForce = AvailableGrip * EffectiveGripMultiplier;

	// Clamp corrective lateral force
	const float AppliedForceMagnitude = FMath::Min(RequiredForceMagnitude, MaxEffectiveForce);
	const float LateralSign = (LateralSpeed > 0.0f) ? -1.0f : 1.0f;

	OutLateralForce = Right * (LateralSign * AppliedForceMagnitude);

	// Drift event dispatching
	const bool bIsDriftingNow = (CurrentDriftAlpha > 0.35f);
	if (bIsDriftingNow && !bWasDrifting)
	{
		OnDriftStarted.Broadcast();
	}
	else if (!bIsDriftingNow && bWasDrifting)
	{
		OnDriftEnded.Broadcast();
	}
	bWasDrifting = bIsDriftingNow;

	return CurrentDriftAlpha;
}
