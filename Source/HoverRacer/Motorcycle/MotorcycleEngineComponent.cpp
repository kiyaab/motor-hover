// Copyright Epic Games, Inc. All Rights Reserved.

#include "MotorcycleEngineComponent.h"
#include "Core/MotorcycleMath.h"

UMotorcycleEngineComponent::UMotorcycleEngineComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UMotorcycleEngineComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bAutomaticTransmission)
	{
		CheckAutomaticShift();
	}
}

float UMotorcycleEngineComponent::GetGearRatio(EMotorcycleGear Gear) const
{
	switch (Gear)
	{
	case EMotorcycleGear::Gear1: return 2.562f;
	case EMotorcycleGear::Gear2: return 2.052f;
	case EMotorcycleGear::Gear3: return 1.714f;
	case EMotorcycleGear::Gear4: return 1.500f;
	case EMotorcycleGear::Gear5: return 1.360f;
	case EMotorcycleGear::Gear6: return 1.269f;
	default: return 0.0f;
	}
}

void UMotorcycleEngineComponent::ShiftUp()
{
	const uint8 CurrentNum = static_cast<uint8>(CurrentGear);
	if (CurrentNum < static_cast<uint8>(EMotorcycleGear::Gear6))
	{
		CurrentGear = static_cast<EMotorcycleGear>(CurrentNum + 1);
		OnGearChanged.Broadcast(CurrentGear);
	}
}

void UMotorcycleEngineComponent::ShiftDown()
{
	const uint8 CurrentNum = static_cast<uint8>(CurrentGear);
	if (CurrentNum > static_cast<uint8>(EMotorcycleGear::Gear1))
	{
		CurrentGear = static_cast<EMotorcycleGear>(CurrentNum - 1);
		OnGearChanged.Broadcast(CurrentGear);
	}
}

void UMotorcycleEngineComponent::CheckAutomaticShift()
{
	if (CurrentGear != EMotorcycleGear::Neutral)
	{
		if (CurrentRPM > 13800.0f && CurrentGear != EMotorcycleGear::Gear6)
		{
			ShiftUp();
		}
		else if (CurrentRPM < 5500.0f && CurrentGear != EMotorcycleGear::Gear1)
		{
			ShiftDown();
		}
	}
}

float UMotorcycleEngineComponent::CalculateRearAxleDriveTorque(float ThrottleInput, float RearWheelAngularVelocity, float DeltaTime)
{
	const float GearRatio = GetGearRatio(CurrentGear);
	const float TotalRatio = GearRatio * PrimaryReductionRatio * FinalDriveRatio;

	if (GearRatio <= 0.0f)
	{
		// Neutral gear
		TargetRPM = FMath::Lerp(IdleRPM, RedlineRPM, ThrottleInput);
		CurrentRPM = FMath::FInterpTo(CurrentRPM, TargetRPM, DeltaTime, 14.0f);
		return 0.0f;
	}

	// Calculate engine RPM from rear wheel rotation: RPM = omega * TotalRatio * (60 / 2pi)
	const float WheelRPM = (RearWheelAngularVelocity * 60.0f) / (2.0f * PI);
	const float WheelDrivenRPM = FMath::Abs(WheelRPM * TotalRatio);

	// Engine RPM matches wheel through engaged clutch, never dropping below idle
	TargetRPM = FMath::Max(IdleRPM, WheelDrivenRPM);
	CurrentRPM = FMath::FInterpTo(CurrentRPM, TargetRPM, DeltaTime, 24.0f);

	// Fetch base engine flywheel torque
	const float EngineTorque = MotorcycleMath::EvaluateEngineTorque(CurrentRPM);

	if (ThrottleInput > 0.05f)
	{
		// Positive driving torque to rear axle
		const float CrankshaftTorque = EngineTorque * ThrottleInput;
		const float AxleTorque = CrankshaftTorque * TotalRatio;
		return AxleTorque;
	}
	else
	{
		// Engine braking on trailing throttle: resists forward wheel rotation
		const float EngineBrakingCrankTorque = -18.0f * (CurrentRPM / RedlineRPM);
		return EngineBrakingCrankTorque * TotalRatio;
	}
}
