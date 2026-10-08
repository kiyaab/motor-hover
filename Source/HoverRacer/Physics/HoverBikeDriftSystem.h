// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Core/HoverBikeTypes.h"
#include "Data/HoverBikeTuningData.h"
#include "HoverBikeDriftSystem.generated.h"

/**
 * Simulates magnetic repulsor lateral traction, momentum-driven drift breakdown,
 * and player-initiated power slides.
 */
UCLASS()
class HOVERRACER_API UHoverBikeDriftSystem : public UObject
{
	GENERATED_BODY()

public:
	UHoverBikeDriftSystem();

	void Initialize(const UHoverBikeTuningData* TuningData);

	/**
	 * Calculates lateral grip correction force opposing sideways slip.
	 *
	 * @param VehicleTransform Vehicle world transform
	 * @param LinearVelocity Center of mass linear velocity (cm/s)
	 * @param VehicleMass Mass in kg
	 * @param GroundedRatio Ratio of grounded repulsors [0, 1]
	 * @param GripEnergyPercent Current repulsor grip energy [0, 100]
	 * @param bDriftInput True if player is holding drift/handbrake
	 * @param DeltaTime Substep delta time
	 * @param OutLateralForce Corrective force vector in Newtons
	 * @return Current drift alpha [0 = full grip, 1 = full slide]
	 */
	float CalculateLateralTractionForce(
		const FTransform& VehicleTransform,
		const FVector& LinearVelocity,
		float VehicleMass,
		float GroundedRatio,
		float GripEnergyPercent,
		bool bDriftInput,
		float DeltaTime,
		FVector& OutLateralForce);

	/** Readouts */
	float GetCurrentDriftAlpha() const { return CurrentDriftAlpha; }
	float GetGripDemandRatio() const { return LastGripDemandRatio; }
	bool IsDrifting() const { return CurrentDriftAlpha > 0.35f; }

	/** Events */
	FOnDriftStartedSignature OnDriftStarted;
	FOnDriftEndedSignature OnDriftEnded;

private:
	UPROPERTY()
	TWeakObjectPtr<const UHoverBikeTuningData> CachedTuningData;

	float CurrentDriftAlpha = 0.0f;
	float LastGripDemandRatio = 0.0f;
	bool bWasDrifting = false;
};
