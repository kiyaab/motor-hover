// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Core/MotorcycleTypes.h"
#include "MotorcycleEngineComponent.generated.h"

/**
 * Simulates a high-performance 1000cc 4-cylinder superbike engine and 6-speed sequential transmission.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HOVERRACER_API UMotorcycleEngineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMotorcycleEngineComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Shifts to the next higher gear */
	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Transmission")
	void ShiftUp();

	/** Shifts to the next lower gear */
	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Transmission")
	void ShiftDown();

	/** Toggles automatic transmission */
	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Transmission")
	void SetAutomaticTransmission(bool bAuto) { bAutomaticTransmission = bAuto; }

	/** Computes net drive torque delivered to the rear wheel axle in Nm */
	float CalculateRearAxleDriveTorque(float ThrottleInput, float RearWheelAngularVelocity, float DeltaTime);

	// Engine readouts
	UFUNCTION(BlueprintPure, Category = "Motorcycle|Engine")
	float GetCurrentRPM() const { return CurrentRPM; }

	UFUNCTION(BlueprintPure, Category = "Motorcycle|Engine")
	EMotorcycleGear GetCurrentGear() const { return CurrentGear; }

	UFUNCTION(BlueprintPure, Category = "Motorcycle|Engine")
	float GetNormalizedRPM() const { return FMath::Clamp((CurrentRPM - IdleRPM) / (MaxRPM - IdleRPM), 0.0f, 1.0f); }

	UFUNCTION(BlueprintPure, Category = "Motorcycle|Engine")
	bool IsRedline() const { return CurrentRPM >= RedlineRPM; }

	UPROPERTY(BlueprintAssignable, Category = "Motorcycle|Events")
	FOnGearChangedSignature OnGearChanged;

	// Configuration
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine|Specs")
	float IdleRPM = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine|Specs")
	float RedlineRPM = 14500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine|Specs")
	float MaxRPM = 15200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine|Transmission")
	bool bAutomaticTransmission = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine|Transmission")
	float FinalDriveRatio = 2.625f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Engine|Transmission")
	float PrimaryReductionRatio = 1.681f;

private:
	EMotorcycleGear CurrentGear = EMotorcycleGear::Gear1;
	float CurrentRPM = 1200.0f;
	float TargetRPM = 1200.0f;
	float ClutchEngagement = 1.0f;

	float GetGearRatio(EMotorcycleGear Gear) const;
	void CheckAutomaticShift();
};
