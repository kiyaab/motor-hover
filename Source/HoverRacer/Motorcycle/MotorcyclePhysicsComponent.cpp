// Copyright Epic Games, Inc. All Rights Reserved.

#include "MotorcyclePhysicsComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "MotorcycleTireComponent.h"
#include "MotorcycleEngineComponent.h"
#include "Core/MotorcycleMath.h"

UMotorcyclePhysicsComponent::UMotorcyclePhysicsComponent()
{
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void UMotorcyclePhysicsComponent::InitializeComponent()
{
	Super::InitializeComponent();

	AActor* Owner = GetOwner();
	if (Owner)
	{
		TArray<UMotorcycleTireComponent*> Tires;
		Owner->GetComponents<UMotorcycleTireComponent>(Tires);
		for (UMotorcycleTireComponent* Tire : Tires)
		{
			if (Tire->bIsFrontWheel)
			{
				FrontWheel = Tire;
			}
			else
			{
				RearWheel = Tire;
			}
		}

		EngineComponent = Owner->FindComponentByClass<UMotorcycleEngineComponent>();
	}
}

void UMotorcyclePhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeltaTime <= 0.0f)
	{
		return;
	}

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UPrimitiveComponent* ChassisMesh = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
	if (ChassisMesh && ChassisMesh->IsSimulatingPhysics())
	{
		SimulateMotorcyclePhysics(DeltaTime, ChassisMesh);
	}
}

void UMotorcyclePhysicsComponent::SimulateMotorcyclePhysics(float DeltaTime, UPrimitiveComponent* ChassisMesh)
{
	if (!ChassisMesh || DeltaTime <= 0.0f || CurrentCrashState != EMotorcycleCrashType::None)
	{
		return;
	}

	const FTransform VehicleTransform = ChassisMesh->GetComponentTransform();
	const FVector LinearVelocity = ChassisMesh->GetPhysicsLinearVelocity();
	const FVector AngularVelocity = ChassisMesh->GetPhysicsAngularVelocityInRadians();
	const float TotalMass = BikeMassKg + RiderMassKg;

	CurrentSpeedKmh = LinearVelocity.Size() * 0.036f;
	const float ForwardSpeedMetersPerSec = FVector::DotProduct(LinearVelocity, VehicleTransform.GetUnitAxis(EAxis::X)) * 0.01f;

	// Longitudinal acceleration for dynamic weight transfer
	CalculatedAcceleration = (LinearVelocity - PreviousVelocity) / DeltaTime;
	PreviousVelocity = LinearVelocity;
	const float LongAccelMetersPerSec2 = FVector::DotProduct(CalculatedAcceleration, VehicleTransform.GetUnitAxis(EAxis::X)) * 0.01f;

	// 1. Dynamic Weight Transfer (N)
	const float DeltaLoadZ = MotorcycleMath::CalculateWeightTransfer(
		TotalMass,
		LongAccelMetersPerSec2,
		CenterOfMassHeightCm * 0.01f,
		WheelbaseCm * 0.01f
	);

	// Static load per wheel (~50/50 balance on superbike)
	const float StaticWheelLoad = (TotalMass * 9.81f) * 0.5f;
	const float FrontDynamicLoad = FMath::Max(0.0f, StaticWheelLoad - DeltaLoadZ);
	const float RearDynamicLoad = FMath::Max(0.0f, StaticWheelLoad + DeltaLoadZ);

	// Wheelie & Stoppie Detection
	const bool bWasWheelie = bIsWheelie;
	bIsWheelie = (FrontDynamicLoad <= 15.0f && ForwardSpeedMetersPerSec > 2.0f && ThrottleInput > 0.4f);
	if (bIsWheelie && !bWasWheelie)
	{
		OnWheelieStarted.Broadcast();
	}

	const bool bWasStoppie = bIsStoppie;
	bIsStoppie = (RearDynamicLoad <= 15.0f && ForwardSpeedMetersPerSec > 5.0f && FrontBrakeInput > 0.6f);
	if (bIsStoppie && !bWasStoppie)
	{
		OnStoppieStarted.Broadcast();
	}

	// 2. Engine & Rear Axle Drive Torque
	float RearDriveTorque = 0.0f;
	if (EngineComponent && RearWheel)
	{
		RearDriveTorque = EngineComponent->CalculateRearAxleDriveTorque(
			ThrottleInput,
			RearWheel->GetWheelState().AngularVelocity,
			DeltaTime
		);
	}

	// 3. Front Wheel Steer Angle (decreases at high speed for stability)
	const float SpeedAlpha = FMath::Clamp(CurrentSpeedKmh / 200.0f, 0.0f, 1.0f);
	const float SteerFactor = FMath::Lerp(1.0f, 0.22f, SpeedAlpha);
	const float EffectiveSteerAngle = SteeringInput * MaxSteeringAngleDeg * SteerFactor;

	// Collision params for tire sweeps
	FCollisionQueryParams QueryParams(TEXT("MotorcycleTires"), false, GetOwner());
	QueryParams.AddIgnoredActor(GetOwner());

	// 4. Update Front Wheel Physics
	FVector FrontSuspForce, FrontFrictionForce, FrontContactLoc;
	if (FrontWheel)
	{
		FrontWheel->UpdateWheelPhysics(
			VehicleTransform,
			LinearVelocity,
			AngularVelocity,
			0.0f, // No drive torque to front
			FrontBrakeInput,
			EffectiveSteerAngle,
			FrontDynamicLoad - StaticWheelLoad,
			bEnableABS,
			DeltaTime,
			GetWorld(),
			QueryParams,
			FrontSuspForce,
			FrontFrictionForce,
			FrontContactLoc
		);

		if (FrontWheel->IsGrounded())
		{
			ChassisMesh->AddForceAtLocation(FrontSuspForce, FrontContactLoc);
			ChassisMesh->AddForceAtLocation(FrontFrictionForce, FrontContactLoc);
		}
	}

	// 5. Update Rear Wheel Physics
	FVector RearSuspForce, RearFrictionForce, RearContactLoc;
	if (RearWheel)
	{
		RearWheel->UpdateWheelPhysics(
			VehicleTransform,
			LinearVelocity,
			AngularVelocity,
			RearDriveTorque,
			RearBrakeInput,
			0.0f, // Rear not steered
			RearDynamicLoad - StaticWheelLoad,
			bEnableABS,
			DeltaTime,
			GetWorld(),
			QueryParams,
			RearSuspForce,
			RearFrictionForce,
			RearContactLoc
		);

		if (RearWheel->IsGrounded())
		{
			ChassisMesh->AddForceAtLocation(RearSuspForce, RearContactLoc);
			ChassisMesh->AddForceAtLocation(RearFrictionForce, RearContactLoc);
		}
	}

	// 6. Dynamic Motorcycle Leaning (atan(v^2 / (r*g)))
	const float LatAccelMeters = FVector::DotProduct(CalculatedAcceleration, VehicleTransform.GetUnitAxis(EAxis::Y)) * 0.01f;
	const float DynamicLeanRad = MotorcycleMath::CalculateMotorcycleLean(LatAccelMeters, 9.81f, MaxLeanAngleDeg);
	const float CommandLeanRad = FMath::DegreesToRadians(SteeringInput * MaxLeanAngleDeg * SpeedAlpha);

	const float TargetRollRad = FMath::Clamp(DynamicLeanRad * 0.45f + CommandLeanRad * 0.55f, -FMath::DegreesToRadians(MaxLeanAngleDeg), FMath::DegreesToRadians(MaxLeanAngleDeg));
	TargetLeanAngleDeg = FMath::RadiansToDegrees(TargetRollRad);

	// Measure current roll relative to road up
	const FVector Right = VehicleTransform.GetUnitAxis(EAxis::Y);
	const float CurrentRollRad = FMath::Asin(FMath::Clamp(FVector::DotProduct(Right, FVector::UpVector), -1.0f, 1.0f));
	CurrentLeanAngleDeg = FMath::RadiansToDegrees(CurrentRollRad);

	const float RollError = TargetRollRad - CurrentRollRad;
	const float RollDamping = FVector::DotProduct(AngularVelocity, VehicleTransform.GetUnitAxis(EAxis::X));

	// Lean torque applied along chassis roll axis
	const float KpLean = 580000.0f;
	const float KdLean = 65000.0f;
	const float LeanTorqueMag = (RollError * KpLean) - (RollDamping * KdLean);
	ChassisMesh->AddTorqueInRadians(VehicleTransform.GetUnitAxis(EAxis::X) * LeanTorqueMag);

	// 7. Aerodynamic Drag (Cd * A * 0.5 * rho * v^2)
	const float SpeedMeters = LinearVelocity.Size() * 0.01f;
	const float AirDensity = 1.225f;
	const float CdA = 0.36f; // Superbike with tucked-in rider
	const float DragForceMag = 0.5f * AirDensity * (SpeedMeters * SpeedMeters) * CdA;
	if (SpeedMeters > 0.5f)
	{
		const FVector DragVector = -LinearVelocity.GetSafeNormal() * DragForceMag;
		ChassisMesh->AddForce(DragVector);
	}

	// 8. Crash Condition Check (Low-Side / High-Side)
	if (FrontWheel && RearWheel && (FrontWheel->IsGrounded() || RearWheel->IsGrounded()))
	{
		const float TotalLatGrip = FrontWheel->GetWheelState().LateralForce + RearWheel->GetWheelState().LateralForce;
		const float AvailableGrip = (FrontDynamicLoad + RearDynamicLoad) * 1.05f;
		CheckCrashConditions(FMath::Abs(TotalLatGrip), AvailableGrip, FMath::Abs(CurrentRollRad));
	}
}

void UMotorcyclePhysicsComponent::CheckCrashConditions(float LateralForceDemand, float AvailableGrip, float LeanAngleRad)
{
	if (LeanAngleRad > FMath::DegreesToRadians(MaxLeanAngleDeg + 6.0f))
	{
		// Over-leaned past footpeg / fairing ground clearance -> Low-side crash
		CurrentCrashState = EMotorcycleCrashType::LowSide;
		OnMotorcycleCrash.Broadcast(CurrentCrashState, CurrentSpeedKmh);
	}
	else if (LateralForceDemand > AvailableGrip * 1.35f && CurrentSpeedKmh > 60.0f)
	{
		// Loss of tire adhesion -> Low-side slide
		CurrentCrashState = EMotorcycleCrashType::LowSide;
		OnMotorcycleCrash.Broadcast(CurrentCrashState, CurrentSpeedKmh);
	}
}
