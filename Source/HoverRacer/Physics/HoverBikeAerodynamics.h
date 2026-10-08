// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Data/HoverBikeTuningData.h"
#include "HoverBikeAerodynamics.generated.h"

/**
 * Simulates high-speed quadratic aerodynamic drag, ground-effect downforce,
 * air brake drag deployment, and active aerodynamic control surfaces.
 */
UCLASS()
class HOVERRACER_API UHoverBikeAerodynamics : public UObject
{
	GENERATED_BODY()

public:
	UHoverBikeAerodynamics();

	void Initialize(const UHoverBikeTuningData* TuningData);

	/**
	 * Calculates total aerodynamic drag force opposing linear velocity.
	 *
	 * @param LinearVelocity Rigid body linear velocity (cm/s)
	 * @param AirBrakeInput Player/AI air brake input [0, 1]
	 * @return Drag force vector in Newtons
	 */
	FVector CalculateDragForce(const FVector& LinearVelocity, float AirBrakeInput) const;

	/**
	 * Calculates dynamic surface-relative downforce.
	 * Clamps bike to track in wall rides, loops, and ceilings without magnetic snapping.
	 *
	 * @param LinearVelocity Rigid body velocity (cm/s)
	 * @param SurfaceNormal Smoothed dynamic surface normal
	 * @param AverageHoverHeight Measured suspension height (cm)
	 * @param GroundedRatio Ratio of grounded repulsors [0, 1]
	 * @return Downforce vector in Newtons (directed toward surface)
	 */
	FVector CalculateDownforce(
		const FVector& LinearVelocity,
		const FVector& SurfaceNormal,
		float AverageHoverHeight,
		float GroundedRatio) const;

	/**
	 * Calculates aerodynamic stabilizing torque produced by active aerodynamic surfaces
	 * (rear stabilizer fin, front canards, air brake flaps) at high speed.
	 */
	FVector CalculateAeroStabilizationTorque(
		const FVector& LinearVelocity,
		const FVector& AngularVelocity,
		const FTransform& VehicleTransform,
		float AirBrakeInput) const;

	/** Active surface deflection setters */
	void SetCanardDeflection(float Amount) { CanardDeflection = FMath::Clamp(Amount, -1.0f, 1.0f); }
	void SetStabilizerDeflection(float Amount) { StabilizerDeflection = FMath::Clamp(Amount, -1.0f, 1.0f); }

	float GetLastDragMagnitude() const { return LastDragMagnitude; }
	float GetLastDownforceMagnitude() const { return LastDownforceMagnitude; }

private:
	UPROPERTY()
	TWeakObjectPtr<const UHoverBikeTuningData> CachedTuningData;

	float CanardDeflection = 0.0f;
	float StabilizerDeflection = 0.0f;

	mutable float LastDragMagnitude = 0.0f;
	mutable float LastDownforceMagnitude = 0.0f;
};
