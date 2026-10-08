// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "HoverBikeCameraComponent.generated.h"

class USpringArmComponent;
class AHoverBike;

/**
 * AAA Cinematic Racing Camera Component.
 * Implements speed-based FOV dilation, drift corner framing, jump pullback, and horizon stabilization.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HOVERRACER_API UHoverBikeCameraComponent : public UCameraComponent
{
	GENERATED_BODY()

public:
	UHoverBikeCameraComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Base FOV at low speed */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|FOV")
	float BaseFOV = 90.0f;

	/** High-speed racing FOV */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|FOV")
	float HighSpeedFOV = 110.0f;

	/** Maximum boost FOV */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|FOV")
	float BoostFOV = 122.0f;

	/** Maximum camera sideways offset during high-speed drifting (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Drift")
	float MaxDriftLateralOffset = 140.0f;

	/** Distance pullback multiplier during airborne jumps */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera|Airborne")
	float AirborneDistanceMultiplier = 1.35f;

private:
	float CurrentTargetFOV = 90.0f;
	float FilteredLateralDriftOffset = 0.0f;
};
