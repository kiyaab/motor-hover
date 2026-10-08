// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/MotorcycleTypes.h"
#include "RiderIKComponent.generated.h"

class USkeletalMeshComponent;
class AMotorcyclePawn;

/**
 * Procedural Rider Kinematics & Animation Controller.
 * Handles rider body hang-off, knee-down cornering, aerodynamic tuck, braking brace, and crash separation.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HOVERRACER_API URiderIKComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	URiderIKComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Rider skeletal mesh reference */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rider")
	TObjectPtr<USkeletalMeshComponent> RiderMesh;

	// Rider procedural animation outputs
	UFUNCTION(BlueprintPure, Category = "Rider|Pose")
	float GetSpineLeanOffset() const { return SpineLeanOffset; }

	UFUNCTION(BlueprintPure, Category = "Rider|Pose")
	float GetKneeExtension() const { return KneeExtension; }

	UFUNCTION(BlueprintPure, Category = "Rider|Pose")
	bool IsTuckedIn() const { return bIsTuckedIn; }

	UFUNCTION(BlueprintPure, Category = "Rider|Pose")
	bool IsBracing() const { return bIsBracing; }

	/** Detaches rider and activates physical ragdoll */
	UFUNCTION(BlueprintCallable, Category = "Rider|Crash")
	void TriggerRiderRagdoll();

private:
	float SpineLeanOffset = 0.0f;
	float KneeExtension = 0.0f;
	bool bIsTuckedIn = false;
	bool bIsBracing = false;
	bool bIsRagdoll = false;
};
