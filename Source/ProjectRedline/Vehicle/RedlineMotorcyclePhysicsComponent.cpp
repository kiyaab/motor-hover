#include "Vehicle/RedlineMotorcyclePhysicsComponent.h"
#include "GameFramework/Actor.h"
#include "Components/PrimitiveComponent.h"

URedlineMotorcyclePhysicsComponent::URedlineMotorcyclePhysicsComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;
}

void URedlineMotorcyclePhysicsComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner)
	{
		UpdatedPrimitive = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
	}
	ResetBikeState();
}

void URedlineMotorcyclePhysicsComponent::ResetBikeState()
{
	Telemetry.SpeedKmh = 0.0f;
	Telemetry.EngineRPM = Tuning.IdleRPM;
	Telemetry.CurrentGear = 1;
	Telemetry.CurrentLeanDeg = 0.0f;
	Telemetry.bIsWheelie = false;
	Telemetry.bIsStoppie = false;
	Telemetry.bIsLowSideSlide = false;
	PreviousLinearVelocity = FVector::ZeroVector;
	CurrentAcceleration = FVector::ZeroVector;
}

void URedlineMotorcyclePhysicsComponent::ShiftUp()
{
	if (Telemetry.CurrentGear < Tuning.ForwardGearRatios.Num())
	{
		Telemetry.CurrentGear++;
		OnGearChanged.Broadcast(Telemetry.CurrentGear);
	}
}

void URedlineMotorcyclePhysicsComponent::ShiftDown()
{
	if (Telemetry.CurrentGear > 1)
	{
		Telemetry.CurrentGear--;
		OnGearChanged.Broadcast(Telemetry.CurrentGear);
	}
}

void URedlineMotorcyclePhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (DeltaTime <= 0.0f || !UpdatedPrimitive)
	{
		return;
	}

	const FVector Velocity = UpdatedPrimitive->GetPhysicsLinearVelocity();
	CurrentAcceleration = (Velocity - PreviousLinearVelocity) / DeltaTime;
	PreviousLinearVelocity = Velocity;

	Telemetry.SpeedKmh = Velocity.Size() * 0.036f; // cm/s to km/h

	UpdateEngineAndGearing(DeltaTime);
	UpdateSuspensionAndWeightShift(DeltaTime);
	UpdateTireFrictionAndForces(DeltaTime);
	UpdateAerodynamics(DeltaTime);
}

void URedlineMotorcyclePhysicsComponent::UpdateEngineAndGearing(float DeltaTime)
{
	const AActor* Owner = GetOwner();
	if (!Owner) return;

	const FVector Forward = Owner->GetActorForwardVector();
	const float ForwardSpeedCmS = FVector::DotProduct(UpdatedPrimitive->GetPhysicsLinearVelocity(), Forward);

	const int32 GearIndex = FMath::Clamp(Telemetry.CurrentGear - 1, 0, Tuning.ForwardGearRatios.Num() - 1);
	const float GearRatio = Tuning.ForwardGearRatios[GearIndex];
	const float TotalRatio = GearRatio * Tuning.PrimaryRatio * Tuning.FinalRatio;

	// Calculate driven RPM from rear wheel linear speed
	const float WheelCircumferenceCm = 2.0f * PI * Tuning.WheelRadiusCm;
	const float WheelRPM = (FMath::Abs(ForwardSpeedCmS) / WheelCircumferenceCm) * 60.0f;
	const float TargetRPM = FMath::Max(Tuning.IdleRPM, WheelRPM * TotalRatio);

	Telemetry.EngineRPM = FMath::FInterpTo(Telemetry.EngineRPM, TargetRPM, DeltaTime, 12.0f);

	// Automatic transmission logic
	if (TransmissionMode == ERedlineTransmissionMode::Automatic)
	{
		if (Telemetry.EngineRPM > Tuning.RedlineRPM - 700.0f && Telemetry.CurrentGear < Tuning.ForwardGearRatios.Num())
		{
			ShiftUp();
		}
		else if (Telemetry.EngineRPM < 5500.0f && Telemetry.CurrentGear > 1)
		{
			ShiftDown();
		}
	}

	// Apply Drive Torque
	if (ThrottleInput > 0.0f && !Telemetry.bIsLowSideSlide)
	{
		// Sinusoidal torque curve approximation peaking at 11,000 RPM
		const float NormRPM = FMath::Clamp(Telemetry.EngineRPM / Tuning.PeakTorqueRPM, 0.0f, 1.4f);
		const float TorqueCurveFactor = FMath::Sin(FMath::Min(1.5707f, NormRPM * 1.5707f));
		const float EngineTorqueNm = TorqueCurveFactor * Tuning.PeakTorqueNm * ThrottleInput;

		const float AxleTorqueNm = EngineTorqueNm * TotalRatio;
		const float DriveForceNewtons = (AxleTorqueNm / (Tuning.WheelRadiusCm * 0.01f));

		// Unreal Engine force is in Newtons * 100 for cm/s^2 acceleration (or kg * cm / s^2)
		const FVector DriveForce = Forward * (DriveForceNewtons * 100.0f);
		UpdatedPrimitive->AddForce(DriveForce);
	}
}

void URedlineMotorcyclePhysicsComponent::UpdateSuspensionAndWeightShift(float DeltaTime)
{
	const AActor* Owner = GetOwner();
	if (!Owner) return;

	const FVector Forward = Owner->GetActorForwardVector();
	const float LongAccel = FVector::DotProduct(CurrentAcceleration, Forward) * 0.01f; // cm/s^2 to m/s^2

	const float TotalMassKg = Tuning.BikeMassKg + Tuning.RiderMassKg;
	const float DeltaLoadN = TotalMassKg * LongAccel * (Tuning.CenterOfMassHeightCm / Tuning.WheelbaseCm);

	const float StaticWheelLoadN = (TotalMassKg * 9.81f) * 0.5f;
	const float FrontLoadN = FMath::Max(0.0f, StaticWheelLoadN - DeltaLoadN);
	const float RearLoadN = FMath::Max(0.0f, StaticWheelLoadN + DeltaLoadN);

	Telemetry.FrontForkCompressionCm = FMath::Clamp(FrontLoadN / Tuning.FrontSpringRateNPerCm, 0.0f, Tuning.FrontSuspensionRestCm);
	Telemetry.RearShockCompressionCm = FMath::Clamp(RearLoadN / Tuning.RearSpringRateNPerCm, 0.0f, Tuning.RearSuspensionRestCm);

	// Wheelie & Stoppie Conditions
	Telemetry.bIsWheelie = (FrontLoadN <= 30.0f && ThrottleInput > 0.2f && Telemetry.SpeedKmh > 5.0f);
	Telemetry.bIsStoppie = (RearLoadN <= 30.0f && FrontBrakeInput > 0.3f && Telemetry.SpeedKmh > 10.0f);
}

void URedlineMotorcyclePhysicsComponent::UpdateTireFrictionAndForces(float DeltaTime)
{
	const AActor* Owner = GetOwner();
	if (!Owner) return;

	const FVector Right = Owner->GetActorRightVector();
	const FVector Up = Owner->GetActorUpVector();
	const FVector Forward = Owner->GetActorForwardVector();

	// Dual-Braking System (Front dual disc + Rear single disc)
	const float FrontBrakeN = (FrontBrakeInput * Tuning.FrontBrakeMaxTorqueNm) / (Tuning.WheelRadiusCm * 0.01f);
	const float RearBrakeN = (RearBrakeInput * Tuning.RearBrakeMaxTorqueNm) / (Tuning.WheelRadiusCm * 0.01f);
	const float TotalBrakeN = FrontBrakeN + RearBrakeN;

	if (TotalBrakeN > 0.0f && Telemetry.SpeedKmh > 0.5f)
	{
		const FVector VelocityDir = UpdatedPrimitive->GetPhysicsLinearVelocity().GetSafeNormal();
		UpdatedPrimitive->AddForce(-VelocityDir * (TotalBrakeN * 100.0f));
	}

	// Dynamic MotoGP Leaning Formula: lean = atan(a_lateral / g)
	const float LatAccel = FVector::DotProduct(CurrentAcceleration, Right) * 0.01f;
	const float SpeedFactor = FMath::Clamp(Telemetry.SpeedKmh / 55.0f, 0.0f, 1.0f);

	const float DynLeanDeg = FMath::RadiansToDegrees(FMath::Atan2(LatAccel, 9.81f));
	const float TargetLeanDeg = -SteeringInput * Tuning.MaxLeanAngleDeg * SpeedFactor;

	Telemetry.CurrentLeanDeg = FMath::FInterpTo(Telemetry.CurrentLeanDeg, TargetLeanDeg, DeltaTime, 8.0f);

	// Yaw Steering Torque & Roll Torque
	const float YawTorque = SteeringInput * 350000.0f * (1.0f - 0.55f * SpeedFactor);
	UpdatedPrimitive->AddTorqueInRadians(Up * YawTorque);

	// Low-Side Over-Lean Detection
	if (FMath::Abs(Telemetry.CurrentLeanDeg) > Tuning.MaxLeanAngleDeg + 3.0f && Telemetry.SpeedKmh > 40.0f)
	{
		if (!Telemetry.bIsLowSideSlide)
		{
			Telemetry.bIsLowSideSlide = true;
			OnLowSideCrash.Broadcast();
		}
	}
}

void URedlineMotorcyclePhysicsComponent::UpdateAerodynamics(float DeltaTime)
{
	const float SpeedMs = Telemetry.SpeedKmh / 3.6f;
	if (SpeedMs <= 0.5f) return;

	const float EffectiveCdA = (Telemetry.SpeedKmh > 160.0f) ? Tuning.TuckedInDragAreaCdA : Tuning.DragAreaCdA;
	const float AirDensity = 1.225f; // kg/m^3
	const float DragForceNewtons = 0.5f * AirDensity * (SpeedMs * SpeedMs) * EffectiveCdA;

	const FVector VelocityDir = UpdatedPrimitive->GetPhysicsLinearVelocity().GetSafeNormal();
	UpdatedPrimitive->AddForce(-VelocityDir * (DragForceNewtons * 100.0f));
}
