// Copyright Epic Games, Inc. All Rights Reserved.

#include "HoverBikeCameraComponent.h"
#include "Vehicle/HoverBike.h"
#include "Vehicle/HoverBikePhysicsComponent.h"
#include "GameFramework/SpringArmComponent.h"

UHoverBikeCameraComponent::UHoverBikeCameraComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;

	bUsePawnControlRotation = false;
	FieldOfView = 90.0f;
}

void UHoverBikeCameraComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AHoverBike* Bike = Cast<AHoverBike>(GetOwner());
	if (!Bike)
	{
		return;
	}

	UHoverBikePhysicsComponent* PhysicsComp = Bike->GetPhysicsComponent();
	if (!PhysicsComp || !PhysicsComp->TuningData)
	{
		return;
	}

	const float SpeedNorm = Bike->GetSpeedNormalized();
	const bool bIsBoosting = (Bike->GetBoostAmount() > 0.05f && SpeedNorm > 0.7f);

	// 1. Dynamic Speed & Boost FOV
	float TargetFOV = FMath::Lerp(BaseFOV, HighSpeedFOV, FMath::Clamp(SpeedNorm, 0.0f, 1.0f));
	if (bIsBoosting)
	{
		TargetFOV = FMath::Lerp(TargetFOV, BoostFOV, 0.8f);
	}
	CurrentTargetFOV = FMath::FInterpTo(CurrentTargetFOV, TargetFOV, DeltaTime, 5.0f);
	SetFieldOfView(CurrentTargetFOV);

	// 2. Drift Corner Framing
	USpringArmComponent* Boom = Cast<USpringArmComponent>(GetAttachParent());
	if (Boom)
	{
		const float DriftAlpha = Bike->GetDriftAmount();
		const float SteerDir = (PhysicsComp->GetStabilization()) ? PhysicsComp->GetStabilization()->GetCurrentLeanAngleDeg() : 0.0f;
		const float DesiredDriftOffset = (SteerDir > 0.0f ? 1.0f : -1.0f) * DriftAlpha * MaxDriftLateralOffset;

		FilteredLateralDriftOffset = FMath::FInterpTo(FilteredLateralDriftOffset, DesiredDriftOffset, DeltaTime, 4.0f);

		// 3. Airborne Pullback
		const bool bIsAirborne = (Bike->GetAirborneAmount() > 0.5f);
		const float TargetBoomLength = bIsAirborne ? (PhysicsComp->TuningData->CameraBoomLength * AirborneDistanceMultiplier) : PhysicsComp->TuningData->CameraBoomLength;
		Boom->TargetArmLength = FMath::FInterpTo(Boom->TargetArmLength, TargetBoomLength, DeltaTime, 3.5f);
		Boom->SocketOffset.Y = FilteredLateralDriftOffset;
	}
}
