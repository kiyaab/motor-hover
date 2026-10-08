// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "Core/MotorcycleTypes.h"
#include "MotorcycleTireComponent.generated.h"

class UPrimitiveComponent;

/**
 * Simulates a real motorcycle wheel, suspension assembly, and tire contact patch.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HOVERRACER_API UMotorcycleTireComponent : public USceneComponent
{
	GENERATED_BODY()

public:
	UMotorcycleTireComponent();

	/** Whether this is the front (steered) wheel or rear (driven) wheel */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel")
	bool bIsFrontWheel = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel")
	FMotorcycleSuspensionConfig SuspensionConfig;

	/** Executes suspension trace and computes tire ground forces */
	void UpdateWheelPhysics(
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
		FVector& OutContactLocation);

	UFUNCTION(BlueprintPure, Category = "Wheel")
	const FMotorcycleWheelState& GetWheelState() const { return WheelState; }

	UFUNCTION(BlueprintPure, Category = "Wheel")
	bool IsGrounded() const { return WheelState.bOnGround; }

	UFUNCTION(BlueprintPure, Category = "Wheel")
	float GetWheelRotationRPM() const { return (WheelState.AngularVelocity * 60.0f) / (2.0f * PI); }

private:
	FMotorcycleWheelState WheelState;
	float PreviousCompression = 0.0f;
};
