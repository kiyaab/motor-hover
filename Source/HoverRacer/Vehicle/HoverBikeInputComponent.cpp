// Copyright Epic Games, Inc. All Rights Reserved.

#include "HoverBikeInputComponent.h"
#include "EnhancedInputComponent.h"

UHoverBikeInputComponent::UHoverBikeInputComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UHoverBikeInputComponent::SetupPlayerInput(UEnhancedInputComponent* EnhancedInputComponent, const UHoverBikeInputConfig* InputConfig)
{
	if (!EnhancedInputComponent || !InputConfig)
	{
		return;
	}

	if (InputConfig->IA_Throttle)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_Throttle, ETriggerEvent::Triggered, this, &UHoverBikeInputComponent::HandleThrottleTriggered);
		EnhancedInputComponent->BindAction(InputConfig->IA_Throttle, ETriggerEvent::Completed, this, &UHoverBikeInputComponent::HandleThrottleCompleted);
	}

	if (InputConfig->IA_Brake)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_Brake, ETriggerEvent::Triggered, this, &UHoverBikeInputComponent::HandleBrakeTriggered);
		EnhancedInputComponent->BindAction(InputConfig->IA_Brake, ETriggerEvent::Completed, this, &UHoverBikeInputComponent::HandleBrakeCompleted);
	}

	if (InputConfig->IA_Steer)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_Steer, ETriggerEvent::Triggered, this, &UHoverBikeInputComponent::HandleSteerTriggered);
		EnhancedInputComponent->BindAction(InputConfig->IA_Steer, ETriggerEvent::Completed, this, &UHoverBikeInputComponent::HandleSteerCompleted);
	}

	if (InputConfig->IA_Drift)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_Drift, ETriggerEvent::Started, this, &UHoverBikeInputComponent::HandleDriftStarted);
		EnhancedInputComponent->BindAction(InputConfig->IA_Drift, ETriggerEvent::Completed, this, &UHoverBikeInputComponent::HandleDriftCompleted);
	}

	if (InputConfig->IA_Boost)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_Boost, ETriggerEvent::Started, this, &UHoverBikeInputComponent::HandleBoostStarted);
		EnhancedInputComponent->BindAction(InputConfig->IA_Boost, ETriggerEvent::Completed, this, &UHoverBikeInputComponent::HandleBoostCompleted);
	}

	if (InputConfig->IA_AirBrake)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_AirBrake, ETriggerEvent::Triggered, this, &UHoverBikeInputComponent::HandleAirBrakeTriggered);
		EnhancedInputComponent->BindAction(InputConfig->IA_AirBrake, ETriggerEvent::Completed, this, &UHoverBikeInputComponent::HandleAirBrakeCompleted);
	}

	if (InputConfig->IA_Recover)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_Recover, ETriggerEvent::Triggered, this, &UHoverBikeInputComponent::HandleRecoverTriggered);
	}

	if (InputConfig->IA_Look)
	{
		EnhancedInputComponent->BindAction(InputConfig->IA_Look, ETriggerEvent::Triggered, this, &UHoverBikeInputComponent::HandleLookTriggered);
	}
}

void UHoverBikeInputComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// Smooth inputs to prevent abrupt physical impulses
	FilteredThrottle = FMath::FInterpTo(FilteredThrottle, RawThrottle, DeltaTime, 14.0f);
	FilteredBrake = FMath::FInterpTo(FilteredBrake, RawBrake, DeltaTime, 16.0f);
	FilteredSteer = FMath::FInterpTo(FilteredSteer, RawSteer, DeltaTime, 18.0f);
	FilteredAirBrake = FMath::FInterpTo(FilteredAirBrake, RawAirBrake, DeltaTime, 12.0f);
}

void UHoverBikeInputComponent::HandleThrottleTriggered(const FInputActionValue& Value)
{
	RawThrottle = Value.Get<float>();
}

void UHoverBikeInputComponent::HandleThrottleCompleted(const FInputActionValue& Value)
{
	RawThrottle = 0.0f;
}

void UHoverBikeInputComponent::HandleBrakeTriggered(const FInputActionValue& Value)
{
	RawBrake = Value.Get<float>();
}

void UHoverBikeInputComponent::HandleBrakeCompleted(const FInputActionValue& Value)
{
	RawBrake = 0.0f;
}

void UHoverBikeInputComponent::HandleSteerTriggered(const FInputActionValue& Value)
{
	RawSteer = Value.Get<float>();
}

void UHoverBikeInputComponent::HandleSteerCompleted(const FInputActionValue& Value)
{
	RawSteer = 0.0f;
}

void UHoverBikeInputComponent::HandleDriftStarted(const FInputActionValue& Value)
{
	bDriftPressed = true;
}

void UHoverBikeInputComponent::HandleDriftCompleted(const FInputActionValue& Value)
{
	bDriftPressed = false;
}

void UHoverBikeInputComponent::HandleBoostStarted(const FInputActionValue& Value)
{
	bBoostPressed = true;
}

void UHoverBikeInputComponent::HandleBoostCompleted(const FInputActionValue& Value)
{
	bBoostPressed = false;
}

void UHoverBikeInputComponent::HandleAirBrakeTriggered(const FInputActionValue& Value)
{
	RawAirBrake = Value.Get<float>();
}

void UHoverBikeInputComponent::HandleAirBrakeCompleted(const FInputActionValue& Value)
{
	RawAirBrake = 0.0f;
}

void UHoverBikeInputComponent::HandleRecoverTriggered(const FInputActionValue& Value)
{
	OnRecoveryRequested.Broadcast();
}

void UHoverBikeInputComponent::HandleLookTriggered(const FInputActionValue& Value)
{
	RawLook = Value.Get<FVector2D>();
}
