// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/HoverBikeTypes.h"
#include "Physics/RepulsorPad.h"
#include "Data/HoverBikeTuningData.h"
#include "RepulsorSystem.generated.h"

class UPrimitiveComponent;

/**
 * Handles multi-point repulsor surface detection, spring-damper suspension physics,
 * dynamic weighted surface normal calculation, and the finite grip energy capacity model.
 */
UCLASS()
class HOVERRACER_API URepulsorSystem : public UObject
{
	GENERATED_BODY()

public:
	URepulsorSystem();

	/** Initializes pad states and cache references */
	void Initialize(const UHoverBikeTuningData* TuningData, UPrimitiveComponent* VehiclePrimitive);

	/**
	 * Performs downward sphere sweeps for each pad along the vehicle's local down vector.
	 * Also calculates predictive offsets for extreme velocity safety.
	 */
	void PerformSurfaceDetection(
		const FTransform& VehicleTransform,
		const FVector& LinearVelocity,
		const FVector& AngularVelocity,
		float DeltaTime,
		UWorld* World,
		const FCollisionQueryParams& QueryParams);

	/**
	 * Computes the damped spring force for each repulsor pad and the world application locations.
	 *
	 * @param VehicleTransform Vehicle world transform
	 * @param LinearVelocity Center of mass velocity (cm/s)
	 * @param AngularVelocity Center of mass angular velocity (rad/s)
	 * @param DeltaTime Substep delta time
	 * @param OutForces Array of force vectors to apply at individual pad locations
	 * @param OutApplicationPoints Array of world locations where forces are applied
	 */
	void CalculateSuspensionForces(
		const FTransform& VehicleTransform,
		const FVector& LinearVelocity,
		const FVector& AngularVelocity,
		float DeltaTime,
		TArray<FVector>& OutForces,
		TArray<FVector>& OutApplicationPoints);

	/** Updates the finite grip energy model */
	void UpdateGripEnergy(float DeltaTime, float GripDemandRatio);

	/** Returns number of currently grounded pads (0 to 4) */
	UFUNCTION(BlueprintPure, Category = "Suspension")
	int32 GetGroundedCount() const { return GroundedPadCount; }

	/** Returns true if at least the minimum required pads have valid contact */
	UFUNCTION(BlueprintPure, Category = "Suspension")
	bool IsGrounded() const { return GroundedPadCount >= 2; }

	/** Returns average current hover height across all valid contacts (cm) */
	UFUNCTION(BlueprintPure, Category = "Suspension")
	float GetAverageHoverHeight() const { return AverageHoverHeight; }

	/** Returns the weighted smoothed surface normal */
	UFUNCTION(BlueprintPure, Category = "Suspension")
	FVector GetSmoothedSurfaceNormal() const { return SmoothedSurfaceNormal; }

	/** Returns current grip energy percentage [0, 100] */
	UFUNCTION(BlueprintPure, Category = "Suspension")
	float GetGripEnergyPercent() const;

	/** Get read-only pad state array */
	const TArray<FRepulsorPadState>& GetPadStates() const { return PadStates; }

	/** Get read-only pad config array */
	const TArray<FRepulsorPadConfig>& GetPadConfigs() const { return PadConfigs; }

	/** Set assisted recovery range boost */
	void SetAssistedRecovery(bool bEnabled) { bAssistedRecoveryActive = bEnabled; }

	/** Event delegates */
	FOnRepulsorContactSignature OnRepulsorContact;
	FOnRepulsorLostSignature OnRepulsorLost;

private:
	UPROPERTY()
	TWeakObjectPtr<const UHoverBikeTuningData> CachedTuningData;

	UPROPERTY()
	TWeakObjectPtr<UPrimitiveComponent> CachedVehiclePrimitive;

	TArray<FRepulsorPadConfig> PadConfigs;
	TArray<FRepulsorPadState> PadStates;

	int32 GroundedPadCount = 0;
	float AverageHoverHeight = 150.0f;

	FVector TargetSurfaceNormal = FVector::UpVector;
	FVector SmoothedSurfaceNormal = FVector::UpVector;
	FVector LastValidSurfaceNormal = FVector::UpVector;

	float CurrentGripEnergy = 100.0f;
	bool bAssistedRecoveryActive = false;
};
