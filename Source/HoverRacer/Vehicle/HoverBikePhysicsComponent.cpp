// Copyright Epic Games, Inc. All Rights Reserved.

#include "HoverBikePhysicsComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Core/HoverBikeMath.h"
#include "Vehicle/HoverBike.h"
#include "Vehicle/HoverBikeInputComponent.h"

UHoverBikePhysicsComponent::UHoverBikePhysicsComponent()
{
	bWantsInitializeComponent = true;
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PrePhysics;

	RepulsorSystem = CreateDefaultSubobject<URepulsorSystem>(TEXT("RepulsorSystem"));
	Aerodynamics = CreateDefaultSubobject<UHoverBikeAerodynamics>(TEXT("Aerodynamics"));
	Stabilization = CreateDefaultSubobject<UHoverBikeStabilization>(TEXT("Stabilization"));
	DriftSystem = CreateDefaultSubobject<UHoverBikeDriftSystem>(TEXT("DriftSystem"));
}

void UHoverBikePhysicsComponent::InitializeComponent()
{
	Super::InitializeComponent();

	if (!TuningData)
	{
		TuningData = NewObject<UHoverBikeTuningData>(this, TEXT("DefaultTuningData"));
	}

	UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(GetOwner()->GetRootComponent());
	if (RepulsorSystem)
	{
		RepulsorSystem->Initialize(TuningData, Primitive);
	}
	if (Aerodynamics)
	{
		Aerodynamics->Initialize(TuningData);
	}
	if (Stabilization)
	{
		Stabilization->Initialize(TuningData);
	}
	if (DriftSystem)
	{
		DriftSystem->Initialize(TuningData);
	}

	if (TuningData)
	{
		BoostEnergy = TuningData->MaxBoostEnergy;
	}
}

void UHoverBikePhysicsComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	if (Owner)
	{
		LastSafeTransform = Owner->GetActorTransform();
		LastSafeSurfaceNormal = Owner->GetActorUpVector();
		LastSafeVelocity = FVector::ZeroVector;
	}
}

void UHoverBikePhysicsComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
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

	UPrimitiveComponent* PrimitiveMesh = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
	if (PrimitiveMesh && PrimitiveMesh->IsSimulatingPhysics())
	{
		SimulatePhysics(DeltaTime, PrimitiveMesh);
	}
}

void UHoverBikePhysicsComponent::SimulatePhysics(float DeltaTime, UPrimitiveComponent* TargetMesh)
{
	if (!TargetMesh || DeltaTime <= 0.0f || !TuningData)
	{
		return;
	}

	AHoverBike* Bike = Cast<AHoverBike>(GetOwner());
	UHoverBikeInputComponent* InputComp = (Bike) ? Bike->GetBikeInputComponent() : nullptr;

	// -------------------------------------------------------------
	// STEP 1: Read Rigid Body State
	// -------------------------------------------------------------
	const FTransform VehicleTransform = TargetMesh->GetComponentTransform();
	const FVector LinearVelocity = TargetMesh->GetPhysicsLinearVelocity();
	const FVector AngularVelocity = TargetMesh->GetPhysicsAngularVelocityInRadians();
	const float Mass = TargetMesh->GetMass();

	CurrentSpeedKmh = LinearVelocity.Size() * 0.036f;
	ForwardSpeedKmh = FVector::DotProduct(LinearVelocity, VehicleTransform.GetUnitAxis(EAxis::X)) * 0.036f;

	// Numerical derivative of acceleration for motorcycle lean formulas
	CalculatedAcceleration = (LinearVelocity - PreviousLinearVelocity) / DeltaTime;
	PreviousLinearVelocity = LinearVelocity;

	// Query Effective Environmental Gravity
	const FVector GravityVector = QueryEffectiveGravity(VehicleTransform.GetLocation());
	const float EffectiveGravityMag = GravityVector.Size();

	// -------------------------------------------------------------
	// STEP 2: Perform Repulsor Surface Detection (Sphere Sweeps)
	// -------------------------------------------------------------
	FCollisionQueryParams QueryParams(TEXT("HoverBikeSuspension"), false, GetOwner());
	QueryParams.AddIgnoredActor(GetOwner());

	RepulsorSystem->PerformSurfaceDetection(
		VehicleTransform,
		LinearVelocity,
		AngularVelocity,
		DeltaTime,
		GetWorld(),
		QueryParams
	);

	// -------------------------------------------------------------
	// STEP 3: Contact & Dynamic Surface Normal
	// -------------------------------------------------------------
	const bool bGrounded = RepulsorSystem->IsGrounded();
	const int32 GroundedCount = RepulsorSystem->GetGroundedCount();
	const float GroundedRatio = static_cast<float>(GroundedCount) / 4.0f;
	const FVector SmoothedNormal = RepulsorSystem->GetSmoothedSurfaceNormal();
	const float AvgHoverHeight = RepulsorSystem->GetAverageHoverHeight();

	if (bGrounded)
	{
		AirborneTime = 0.0f;
	}
	else
	{
		AirborneTime += DeltaTime;
	}

	// -------------------------------------------------------------
	// STEP 4: Apply Repulsor Spring Forces (Force-at-Location)
	// -------------------------------------------------------------
	TArray<FVector> SuspensionForces;
	TArray<FVector> ForceLocations;
	RepulsorSystem->CalculateSuspensionForces(
		VehicleTransform,
		LinearVelocity,
		AngularVelocity,
		DeltaTime,
		SuspensionForces,
		ForceLocations
	);

	for (int32 i = 0; i < SuspensionForces.Num(); ++i)
	{
		if (!SuspensionForces[i].IsNearlyZero())
		{
			// Critical: Applied at actual repulsor world locations to generate natural pitch/roll weight transfer
			TargetMesh->AddForceAtLocation(SuspensionForces[i], ForceLocations[i]);
		}
	}

	// -------------------------------------------------------------
	// STEP 5: Surface-Relative Stabilization Torque (PD Alignment)
	// -------------------------------------------------------------
	const FVector AlignmentTorque = Stabilization->CalculateSurfaceAlignmentTorque(
		VehicleTransform,
		SmoothedNormal,
		AngularVelocity,
		bGrounded,
		AirborneTime
	);
	TargetMesh->AddTorqueInRadians(AlignmentTorque);

	// -------------------------------------------------------------
	// STEP 6 & 7: Steering Dynamics & Dynamic Motorcycle Lean
	// -------------------------------------------------------------
	const float SteerInput = (InputComp) ? InputComp->GetSteeringInput() : 0.0f;
	FVector SteerTorque = FVector::ZeroVector;
	FVector LeanTorque = FVector::ZeroVector;

	Stabilization->CalculateSteeringAndLeanTorque(
		VehicleTransform,
		LinearVelocity,
		AngularVelocity,
		CalculatedAcceleration,
		EffectiveGravityMag,
		SteerInput,
		DeltaTime,
		SteerTorque,
		LeanTorque
	);

	LastSteeringTorque = SteerTorque;
	LastLeanTorque = LeanTorque;

	// Scale steering torque by grounded authority
	const float SteerAuthority = bGrounded ? 1.0f : 0.25f;
	TargetMesh->AddTorqueInRadians(SteerTorque * SteerAuthority);
	TargetMesh->AddTorqueInRadians(LeanTorque * SteerAuthority);

	// -------------------------------------------------------------
	// STEP 8: Lateral Grip & Momentum Drift Forces
	// -------------------------------------------------------------
	const bool bDriftInput = (InputComp) ? InputComp->IsDriftPressed() : false;
	const float GripEnergy = RepulsorSystem->GetGripEnergyPercent();
	FVector LateralGripForce = FVector::ZeroVector;

	const float DriftAlpha = DriftSystem->CalculateLateralTractionForce(
		VehicleTransform,
		LinearVelocity,
		Mass,
		GroundedRatio,
		GripEnergy,
		bDriftInput,
		DeltaTime,
		LateralGripForce
	);

	RepulsorSystem->UpdateGripEnergy(DeltaTime, DriftSystem->GetGripDemandRatio());

	if (!LateralGripForce.IsNearlyZero())
	{
		TargetMesh->AddForce(LateralGripForce);
	}

	// -------------------------------------------------------------
	// STEP 9: Forward & Reverse Propulsion Thrusters
	// -------------------------------------------------------------
	const float RawThrottle = (InputComp) ? InputComp->GetThrottleInput() : 0.0f;
	const bool bBoostInput = (InputComp) ? InputComp->IsBoostPressed() : false;

	// Boost reservoir management
	bool bBoosting = false;
	if (bBoostInput && BoostEnergy > 0.0f && RawThrottle > 0.1f)
	{
		bBoosting = true;
		BoostEnergy = FMath::Max(0.0f, BoostEnergy - (TuningData->BoostConsumptionRate * DeltaTime));
	}
	else
	{
		BoostEnergy = FMath::Min(TuningData->MaxBoostEnergy, BoostEnergy + (TuningData->BoostRechargeRate * DeltaTime));
	}

	const float NormalizedSpeed = CurrentSpeedKmh / FMath::Max(1.0f, TuningData->MaxSpeedKmh);
	const float PowerMultiplier = HoverBikeMath::EvaluateEnginePowerCurve(NormalizedSpeed);

	float ThrustMagnitude = 0.0f;
	if (RawThrottle >= 0.0f)
	{
		ThrustMagnitude = RawThrottle * TuningData->MaxForwardThrust * PowerMultiplier;
		if (bBoosting)
		{
			ThrustMagnitude *= TuningData->BoostMultiplier;
		}
	}
	else
	{
		ThrustMagnitude = RawThrottle * TuningData->MaxReverseThrust;
	}

	LastThrustApplied = ThrustMagnitude;
	const FVector ForwardVector = VehicleTransform.GetUnitAxis(EAxis::X);
	const FVector ThrustForce = ForwardVector * ThrustMagnitude;

	// Thrusters drive the bike forward even when drifting
	TargetMesh->AddForce(ThrustForce);

	// -------------------------------------------------------------
	// STEP 10: Quadratic Aerodynamic Drag
	// -------------------------------------------------------------
	const float AirBrakeInput = (InputComp) ? InputComp->GetAirBrakeInput() : 0.0f;
	const FVector DragForce = Aerodynamics->CalculateDragForce(LinearVelocity, AirBrakeInput);
	LastDragApplied = DragForce.Size();
	TargetMesh->AddForce(DragForce);

	// -------------------------------------------------------------
	// STEP 11: Dynamic Ground-Effect Downforce
	// -------------------------------------------------------------
	const FVector Downforce = Aerodynamics->CalculateDownforce(LinearVelocity, SmoothedNormal, AvgHoverHeight, GroundedRatio);
	LastDownforceApplied = Downforce.Size();
	TargetMesh->AddForce(Downforce);

	// -------------------------------------------------------------
	// STEP 12: Air Braking Torques & Control Surface Aerodynamics
	// -------------------------------------------------------------
	const FVector AeroTorque = Aerodynamics->CalculateAeroStabilizationTorque(
		LinearVelocity,
		AngularVelocity,
		VehicleTransform,
		AirBrakeInput
	);
	TargetMesh->AddTorqueInRadians(AeroTorque);

	// -------------------------------------------------------------
	// STEP 13: Gyroscopic Angular Damping
	// -------------------------------------------------------------
	const FVector GyroTorque = Stabilization->CalculateGyroscopicTorque(
		VehicleTransform,
		AngularVelocity,
		SteerInput
	);
	TargetMesh->AddTorqueInRadians(GyroTorque);

	// -------------------------------------------------------------
	// STEP 14: State Machine & Recovery Check
	// -------------------------------------------------------------
	UpdateStateMachine(bGrounded, GroundedCount, SmoothedNormal, VehicleTransform);
	UpdateSafeTrackState(DeltaTime, VehicleTransform, LinearVelocity, SmoothedNormal, bGrounded);
	CheckCrashConditions(LinearVelocity, AngularVelocity, DeltaTime, TargetMesh);

	// Store serialized snapshot
	StateSnapshot.Position = VehicleTransform.GetLocation();
	StateSnapshot.Rotation = VehicleTransform.GetRotation();
	StateSnapshot.LinearVelocity = LinearVelocity;
	StateSnapshot.AngularVelocity = AngularVelocity;
	StateSnapshot.SurfaceNormal = SmoothedNormal;
	StateSnapshot.Throttle = RawThrottle;
	StateSnapshot.Steering = SteerInput;
	StateSnapshot.Drift = DriftAlpha;
	StateSnapshot.GroundedPads = static_cast<uint8>(GroundedCount);
}

void UHoverBikePhysicsComponent::UpdateStateMachine(
	bool bGrounded,
	int32 GroundedCount,
	const FVector& SurfaceNormal,
	const FTransform& VehicleTransform)
{
	if (bIsRecovering)
	{
		CurrentState = EHoverBikeState::Recovering;
		return;
	}

	if (!bGrounded)
	{
		CurrentState = EHoverBikeState::Airborne;
		return;
	}

	if (GroundedCount < 4)
	{
		CurrentState = EHoverBikeState::PartialContact;
	}
	else if (DriftSystem->IsDrifting())
	{
		CurrentState = EHoverBikeState::Drifting;
	}
	else
	{
		// Surface orientation classification
		const float NormalZ = SurfaceNormal.Z;
		if (NormalZ < -0.3f)
		{
			CurrentState = EHoverBikeState::CeilingRide;
		}
		else if (FMath::Abs(NormalZ) <= 0.35f)
		{
			CurrentState = EHoverBikeState::WallRide;
		}
		else
		{
			CurrentState = EHoverBikeState::Grounded;
		}
	}
}

void UHoverBikePhysicsComponent::UpdateSafeTrackState(
	float DeltaTime,
	const FTransform& VehicleTransform,
	const FVector& Velocity,
	const FVector& SurfaceNormal,
	bool bGrounded)
{
	if (bGrounded && CurrentState != EHoverBikeState::Crashing && CurrentState != EHoverBikeState::Recovering)
	{
		TimeSinceLastSafe = 0.0f;
		LastSafeTransform = VehicleTransform;
		LastSafeSurfaceNormal = SurfaceNormal;
		LastSafeVelocity = Velocity;
		InversionTimer = 0.0f;
	}
	else
	{
		TimeSinceLastSafe += DeltaTime;

		// Check if bike is upside down relative to surface
		const float UpDot = FVector::DotProduct(VehicleTransform.GetUnitAxis(EAxis::Z), SurfaceNormal);
		if (UpDot < 0.0f)
		{
			InversionTimer += DeltaTime;
			if (InversionTimer > TuningData->InversionTimeout && !bIsRecovering)
			{
				TriggerRecovery();
			}
		}
		else
		{
			InversionTimer = 0.0f;
		}
	}
}

void UHoverBikePhysicsComponent::TriggerRecovery()
{
	if (bIsRecovering)
	{
		return;
	}

	bIsRecovering = true;
	RecoveryTimer = 0.0f;

	// Soft recovery: Assisted repulsor range boost + upright torque
	RepulsorSystem->SetAssistedRecovery(true);

	AActor* Owner = GetOwner();
	if (!Owner)
	{
		return;
	}

	UPrimitiveComponent* Mesh = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
	if (Mesh)
	{
		// Soft upright impulse
		const FVector CurrentUp = Owner->GetActorUpVector();
		const FVector DesiredUp = LastSafeSurfaceNormal;
		const FVector CorrectionAxis = FVector::CrossProduct(CurrentUp, DesiredUp);
		Mesh->AddTorqueInRadians(CorrectionAxis * TuningData->SoftRecoveryTorque);

		// If falling uncontrollably for too long (> 3.5s), execute full checkpoint reset
		if (TimeSinceLastSafe > 3.5f)
		{
			Mesh->SetPhysicsLinearVelocity(LastSafeVelocity * 0.5f);
			Mesh->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
			Owner->SetActorTransform(LastSafeTransform, false, nullptr, ETeleportType::TeleportPhysics);
		}
	}

	FTimerHandle RecoveryTimerHandle;
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().SetTimer(
			RecoveryTimerHandle,
			[this]()
			{
				bIsRecovering = false;
				if (RepulsorSystem)
				{
					RepulsorSystem->SetAssistedRecovery(false);
				}
			},
			1.2f,
			false
		);
	}
}

void UHoverBikePhysicsComponent::CheckCrashConditions(
	const FVector& LinearVelocity,
	const FVector& AngularVelocity,
	float DeltaTime,
	UPrimitiveComponent* TargetMesh)
{
	// Calculate kinetic rotational & impact energy
	const float Speed = LinearVelocity.Size();
	const float AngularSpeed = AngularVelocity.Size();
	const float KineticRotEnergy = 0.5f * TargetMesh->GetMass() * (Speed * 0.01f) * (Speed * 0.01f);

	if (KineticRotEnergy > TuningData->CrashImpactThreshold && AngularSpeed > 10.0f)
	{
		CurrentState = EHoverBikeState::Crashing;
		AHoverBike* Bike = Cast<AHoverBike>(GetOwner());
		if (Bike)
		{
			Bike->OnCrash.Broadcast(KineticRotEnergy, TargetMesh->GetComponentLocation());
		}
	}
}

FVector UHoverBikePhysicsComponent::QueryEffectiveGravity(const FVector& WorldLocation) const
{
	// Query custom gravity volumes implementing IGravityProviderInterface
	UWorld* World = GetWorld();
	if (!World)
	{
		return FVector(0.0f, 0.0f, -980.0f);
	}

	// Default engine world gravity
	return FVector(0.0f, 0.0f, World->GetGravityZ());
}

float UHoverBikePhysicsComponent::GetBoostEnergyPercent() const
{
	return (TuningData && TuningData->MaxBoostEnergy > 0.0f) ? (BoostEnergy / TuningData->MaxBoostEnergy) * 100.0f : 100.0f;
}

float UHoverBikePhysicsComponent::GetGripEnergyPercent() const
{
	return RepulsorSystem ? RepulsorSystem->GetGripEnergyPercent() : 100.0f;
}

float UHoverBikePhysicsComponent::GetCurrentDriftAlpha() const
{
	return DriftSystem ? DriftSystem->GetCurrentDriftAlpha() : 0.0f;
}

float UHoverBikePhysicsComponent::GetHoverHeight() const
{
	return RepulsorSystem ? RepulsorSystem->GetAverageHoverHeight() : 150.0f;
}

int32 UHoverBikePhysicsComponent::GetGroundedPadCount() const
{
	return RepulsorSystem ? RepulsorSystem->GetGroundedCount() : 0;
}

FVector UHoverBikePhysicsComponent::GetSmoothedSurfaceNormal() const
{
	return RepulsorSystem ? RepulsorSystem->GetSmoothedSurfaceNormal() : FVector::UpVector;
}
