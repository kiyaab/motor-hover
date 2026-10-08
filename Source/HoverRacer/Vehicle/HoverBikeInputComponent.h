// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InputActionValue.h"
#include "Input/HoverBikeInputConfig.h"
#include "HoverBikeInputComponent.generated.h"

/**
 * Handles Enhanced Input event dispatching, player control smoothing,
 * and AI input injection. Decoupled from physics execution.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HOVERRACER_API UHoverBikeInputComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHoverBikeInputComponent();

	virtual void SetupPlayerInput(class UEnhancedInputComponent* EnhancedInputComponent, const UHoverBikeInputConfig* InputConfig);

	// Input consumption accessors (called by physics loop)
	UFUNCTION(BlueprintPure, Category = "HoverBike|Input")
	float GetThrottleInput() const { return FilteredThrottle; }

	UFUNCTION(BlueprintPure, Category = "HoverBike|Input")
	float GetBrakeInput() const { return FilteredBrake; }

	UFUNCTION(BlueprintPure, Category = "HoverBike|Input")
	float GetSteeringInput() const { return FilteredSteer; }

	UFUNCTION(BlueprintPure, Category = "HoverBike|Input")
	bool IsDriftPressed() const { return bDriftPressed; }

	UFUNCTION(BlueprintPure, Category = "HoverBike|Input")
	bool IsBoostPressed() const { return bBoostPressed; }

	UFUNCTION(BlueprintPure, Category = "HoverBike|Input")
	float GetAirBrakeInput() const { return FilteredAirBrake; }

	UFUNCTION(BlueprintPure, Category = "HoverBike|Input")
	FVector2D GetLookInput() const { return RawLook; }

	// Direct input injection interface for AI & Replay
	UFUNCTION(BlueprintCallable, Category = "HoverBike|Input")
	void SetThrottleInput(float Value) { RawThrottle = FMath::Clamp(Value, -1.0f, 1.0f); }

	UFUNCTION(BlueprintCallable, Category = "HoverBike|Input")
	void SetBrakeInput(float Value) { RawBrake = FMath::Clamp(Value, 0.0f, 1.0f); }

	UFUNCTION(BlueprintCallable, Category = "HoverBike|Input")
	void SetSteeringInput(float Value) { RawSteer = FMath::Clamp(Value, -1.0f, 1.0f); }

	UFUNCTION(BlueprintCallable, Category = "HoverBike|Input")
	void SetDriftInput(bool bActive) { bDriftPressed = bActive; }

	UFUNCTION(BlueprintCallable, Category = "HoverBike|Input")
	void SetBoostInput(bool bActive) { bBoostPressed = bActive; }

	UFUNCTION(BlueprintCallable, Category = "HoverBike|Input")
	void SetAirBrakeInput(float Value) { RawAirBrake = FMath::Clamp(Value, 0.0f, 1.0f); }

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Delegate for recovery action request */
	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRecoveryRequestedSignature);
	UPROPERTY(BlueprintAssignable, Category = "HoverBike|Events")
	FOnRecoveryRequestedSignature OnRecoveryRequested;

private:
	// Raw input targets
	float RawThrottle = 0.0f;
	float RawBrake = 0.0f;
	float RawSteer = 0.0f;
	float RawAirBrake = 0.0f;
	bool bDriftPressed = false;
	bool bBoostPressed = false;
	FVector2D RawLook = FVector2D::ZeroVector;

	// Filtered smoothed inputs
	float FilteredThrottle = 0.0f;
	float FilteredBrake = 0.0f;
	float FilteredSteer = 0.0f;
	float FilteredAirBrake = 0.0f;

	// Action callbacks
	void HandleThrottleTriggered(const FInputActionValue& Value);
	void HandleThrottleCompleted(const FInputActionValue& Value);

	void HandleBrakeTriggered(const FInputActionValue& Value);
	void HandleBrakeCompleted(const FInputActionValue& Value);

	void HandleSteerTriggered(const FInputActionValue& Value);
	void HandleSteerCompleted(const FInputActionValue& Value);

	void HandleDriftStarted(const FInputActionValue& Value);
	void HandleDriftCompleted(const FInputActionValue& Value);

	void HandleBoostStarted(const FInputActionValue& Value);
	void HandleBoostCompleted(const FInputActionValue& Value);

	void HandleAirBrakeTriggered(const FInputActionValue& Value);
	void HandleAirBrakeCompleted(const FInputActionValue& Value);

	void HandleRecoverTriggered(const FInputActionValue& Value);
	void HandleLookTriggered(const FInputActionValue& Value);
};
