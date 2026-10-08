// Copyright Epic Games, Inc. All Rights Reserved.

#include "RepulsorSystem.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/World.h"
#include "Core/HoverBikeMath.h"

URepulsorSystem::URepulsorSystem()
{
	PadConfigs.SetNum(HOVERBIKE_REPULSOR_COUNT);
	PadStates.SetNum(HOVERBIKE_REPULSOR_COUNT);
}

void URepulsorSystem::Initialize(const UHoverBikeTuningData* TuningData, UPrimitiveComponent* VehiclePrimitive)
{
	CachedTuningData = TuningData;
	CachedVehiclePrimitive = VehiclePrimitive;

	if (TuningData)
	{
		PadConfigs = TuningData->RepulsorConfigs;
		CurrentGripEnergy = TuningData->MaxGripEnergy;
		AverageHoverHeight = TuningData->HoverHeight;
	}

	PadStates.SetNum(PadConfigs.Num());
	for (int32 i = 0; i < PadStates.Num(); ++i)
	{
		PadStates[i] = FRepulsorPadState();
		PadStates[i].CurrentLength = (i < PadConfigs.Num()) ? PadConfigs[i].RestLength : 150.0f;
	}

	SmoothedSurfaceNormal = FVector::UpVector;
	TargetSurfaceNormal = FVector::UpVector;
	LastValidSurfaceNormal = FVector::UpVector;
}

void URepulsorSystem::PerformSurfaceDetection(
	const FTransform& VehicleTransform,
	const FVector& LinearVelocity,
	const FVector& AngularVelocity,
	float DeltaTime,
	UWorld* World,
	const FCollisionQueryParams& QueryParams)
{
	if (!World)
	{
		return;
	}

	const UHoverBikeTuningData* Tuning = CachedTuningData.Get();
	const float RangeMultiplier = (bAssistedRecoveryActive && Tuning) ? Tuning->AssistedRecoveryRangeMultiplier : 1.0f;

	// Critical: Local Down vector relative to vehicle orientation, never world Z!
	const FVector LocalDown = -VehicleTransform.GetUnitAxis(EAxis::Z);

	int32 NewGroundedCount = 0;
	float TotalGroundedHeight = 0.0f;
	FVector WeightedNormalSum = FVector::ZeroVector;
	float TotalNormalWeight = 0.0f;

	for (int32 i = 0; i < PadConfigs.Num(); ++i)
	{
		const FRepulsorPadConfig& Config = PadConfigs[i];
		FRepulsorPadState& State = PadStates[i];

		const FVector MountWorldPos = VehicleTransform.TransformPosition(Config.LocalOffset);
		const float EffectiveMaxTrace = Config.MaxTraceLength * RangeMultiplier;

		// Predictive sweep offset for ultra-high velocity stability
		const FVector VelocityLead = LinearVelocity * (DeltaTime * 0.5f);
		const FVector TraceStart = MountWorldPos;
		const FVector TraceEnd = MountWorldPos + (LocalDown * EffectiveMaxTrace) + VelocityLead;

		FHitResult HitResult;
		const FCollisionShape SphereShape = FCollisionShape::MakeSphere(Config.TraceRadius);

		const bool bHit = World->SweepSingleByChannel(
			HitResult,
			TraceStart,
			TraceEnd,
			VehicleTransform.GetRotation(),
			ECC_WorldStatic,
			SphereShape,
			QueryParams
		);

		const bool bWasGrounded = State.bGrounded;

		if (bHit && HitResult.bBlockingHit && HitResult.Distance > 0.0f)
		{
			State.bGrounded = true;
			State.TimeSinceGrounded = 0.0f;

			// Distance projected along local down
			const float HitDistance = FMath::Max(0.0f, FVector::DotProduct(HitResult.Location - MountWorldPos, LocalDown));
			const float PreviousLength = State.CurrentLength;
			State.CurrentLength = HitDistance;
			State.Compression = Config.RestLength - State.CurrentLength;
			State.CompressionVelocity = (DeltaTime > 0.0f) ? (PreviousLength - State.CurrentLength) / DeltaTime : 0.0f;
			State.HitLocation = HitResult.Location;
			State.SurfaceNormal = HoverBikeMath::SafeNormalize(HitResult.ImpactNormal, VehicleTransform.GetUnitAxis(EAxis::Z));

			NewGroundedCount++;
			TotalGroundedHeight += State.CurrentLength;

			// Weighted normal accumulation
			const float CompressionWeight = FMath::Max(0.1f, State.Compression + 50.0f);
			const float Weight = CompressionWeight * Config.SurfaceNormalWeight;
			WeightedNormalSum += State.SurfaceNormal * Weight;
			TotalNormalWeight += Weight;

			if (!bWasGrounded)
			{
				OnRepulsorContact.Broadcast(i, State.HitLocation);
			}
		}
		else
		{
			State.TimeSinceGrounded += DeltaTime;
			if (State.TimeSinceGrounded > Config.ContactGraceTime)
			{
				State.bGrounded = false;
				State.Compression = 0.0f;
				State.CompressionVelocity = 0.0f;
				State.CurrentLength = EffectiveMaxTrace;

				if (bWasGrounded)
				{
					OnRepulsorLost.Broadcast(i);
				}
			}
		}
	}

	GroundedPadCount = NewGroundedCount;

	if (GroundedPadCount > 0)
	{
		AverageHoverHeight = TotalGroundedHeight / static_cast<float>(GroundedPadCount);
		if (TotalNormalWeight > 0.0f)
		{
			TargetSurfaceNormal = HoverBikeMath::SafeNormalize(WeightedNormalSum, LastValidSurfaceNormal);
			LastValidSurfaceNormal = TargetSurfaceNormal;
		}
	}
	else
	{
		// Airborne: target smoothly decays toward last valid normal or gravity up
		TargetSurfaceNormal = LastValidSurfaceNormal;
	}

	// Smooth surface normal transition using quaternion slerp or vector interpolation
	const float SmoothSpeed = (Tuning) ? Tuning->SurfaceNormalSmoothingRate : 18.0f;
	SmoothedSurfaceNormal = FMath::VInterpTo(SmoothedSurfaceNormal, TargetSurfaceNormal, DeltaTime, SmoothSpeed);
	SmoothedSurfaceNormal = HoverBikeMath::SafeNormalize(SmoothedSurfaceNormal, FVector::UpVector);
}

void URepulsorSystem::CalculateSuspensionForces(
	const FTransform& VehicleTransform,
	const FVector& LinearVelocity,
	const FVector& AngularVelocity,
	float DeltaTime,
	TArray<FVector>& OutForces,
	TArray<FVector>& OutApplicationPoints)
{
	OutForces.Reset(PadConfigs.Num());
	OutApplicationPoints.Reset(PadConfigs.Num());

	const UHoverBikeTuningData* Tuning = CachedTuningData.Get();
	const float StiffnessMultiplier = (Tuning) ? Tuning->GlobalSpringStiffnessMultiplier : 1.0f;
	const float DampingMultiplier = (Tuning) ? Tuning->GlobalDampingMultiplier : 1.0f;

	for (int32 i = 0; i < PadConfigs.Num(); ++i)
	{
		const FRepulsorPadConfig& Config = PadConfigs[i];
		FRepulsorPadState& State = PadStates[i];

		const FVector MountWorldPos = VehicleTransform.TransformPosition(Config.LocalOffset);
		OutApplicationPoints.Add(MountWorldPos);

		if (!State.bGrounded)
		{
			State.AppliedForce = 0.0f;
			OutForces.Add(FVector::ZeroVector);
			continue;
		}

		// Calculate exact rigid body point velocity at repulsor mount
		const FVector RelativePos = MountWorldPos - VehicleTransform.GetLocation();
		const FVector PointVelocity = HoverBikeMath::CalculatePointVelocity(LinearVelocity, AngularVelocity, RelativePos);
		State.ContactVelocity = PointVelocity;

		// Velocity projected onto surface normal
		const float VelocityAlongNormal = FVector::DotProduct(PointVelocity, State.SurfaceNormal);

		// Calculate damped spring force
		const float ForceMagnitude = HoverBikeMath::CalculateDampedSpringForce(
			Config.RestLength,
			State.CurrentLength,
			Config.SpringStiffness * StiffnessMultiplier,
			Config.DampingCoefficient * DampingMultiplier,
			VelocityAlongNormal,
			Config.MaxForce
		);

		State.AppliedForce = ForceMagnitude;

		// Suspension pushes directly away from the surface along the contact normal
		const FVector SuspensionForceVector = State.SurfaceNormal * ForceMagnitude;
		OutForces.Add(SuspensionForceVector);
	}
}

void URepulsorSystem::UpdateGripEnergy(float DeltaTime, float GripDemandRatio)
{
	const UHoverBikeTuningData* Tuning = CachedTuningData.Get();
	if (!Tuning)
	{
		return;
	}

	if (GripDemandRatio > Tuning->DriftStartThreshold)
	{
		// Consume grip energy proportionally to overload
		const float Overload = (GripDemandRatio - Tuning->DriftStartThreshold);
		CurrentGripEnergy = FMath::Max(0.0f, CurrentGripEnergy - (Tuning->GripConsumptionRate * Overload * DeltaTime));
	}
	else
	{
		// Recover grip energy
		CurrentGripEnergy = FMath::Min(Tuning->MaxGripEnergy, CurrentGripEnergy + (Tuning->GripRecoveryRate * DeltaTime));
	}
}

float URepulsorSystem::GetGripEnergyPercent() const
{
	const UHoverBikeTuningData* Tuning = CachedTuningData.Get();
	const float MaxEnergy = (Tuning && Tuning->MaxGripEnergy > 0.0f) ? Tuning->MaxGripEnergy : 100.0f;
	return FMath::Clamp((CurrentGripEnergy / MaxEnergy) * 100.0f, 0.0f, 100.0f);
}
