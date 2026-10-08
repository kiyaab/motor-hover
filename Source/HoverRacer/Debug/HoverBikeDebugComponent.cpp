// Copyright Epic Games, Inc. All Rights Reserved.

#include "HoverBikeDebugComponent.h"
#include "DrawDebugHelpers.h"
#include "Engine/Engine.h"
#include "Vehicle/HoverBike.h"
#include "Vehicle/HoverBikePhysicsComponent.h"
#include "Physics/RepulsorSystem.h"
#include "Physics/HoverBikeStabilization.h"
#include "Core/HoverBikeDebug.h"

UHoverBikeDebugComponent::UHoverBikeDebugComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void UHoverBikeDebugComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	const AHoverBike* Bike = Cast<AHoverBike>(GetOwner());
	if (!Bike)
	{
		return;
	}

	const UHoverBikePhysicsComponent* PhysicsComp = Bike->GetPhysicsComponent();
	if (!PhysicsComp)
	{
		return;
	}

	if (bShowDebugVisuals)
	{
		Draw3DVisuals(Bike, PhysicsComp);
	}

	if (bShowDebugHUD)
	{
		DrawScreenHUD(Bike, PhysicsComp);
	}
}

void UHoverBikeDebugComponent::Draw3DVisuals(const AHoverBike* Bike, const UHoverBikePhysicsComponent* PhysicsComp)
{
	UWorld* World = GetWorld();
	if (!World || !Bike->GetBikeMesh())
	{
		return;
	}

	const FTransform VehicleTransform = Bike->GetActorTransform();
	const FVector VehicleLocation = VehicleTransform.GetLocation();
	const FVector LocalDown = -VehicleTransform.GetUnitAxis(EAxis::Z);

	const URepulsorSystem* RepulsorSystem = PhysicsComp->GetRepulsorSystem();
	if (RepulsorSystem)
	{
		const TArray<FRepulsorPadConfig>& Configs = RepulsorSystem->GetPadConfigs();
		const TArray<FRepulsorPadState>& States = RepulsorSystem->GetPadStates();

		for (int32 i = 0; i < Configs.Num(); ++i)
		{
			if (i >= States.Num())
			{
				break;
			}

			const FRepulsorPadConfig& Config = Configs[i];
			const FRepulsorPadState& State = States[i];
			const FVector MountPos = VehicleTransform.TransformPosition(Config.LocalOffset);

			// Repulsor trace ray / sphere
			if (State.bGrounded)
			{
				const float CompressionRatio = State.Compression / Config.RestLength;
				// Green if healthy, Yellow if near bottoming out
				const FColor TraceColor = (CompressionRatio > 0.65f) ? FColor::Yellow : FColor::Green;

				DrawDebugLine(World, MountPos, State.HitLocation, TraceColor, false, -1.0f, 0, 2.0f);
				DrawDebugSphere(World, State.HitLocation, Config.TraceRadius * 0.5f, 8, TraceColor, false, -1.0f, 0, 1.5f);

				// Surface normal blue arrow
				DrawDebugDirectionalArrow(World, State.HitLocation, State.HitLocation + (State.SurfaceNormal * 40.0f), 10.0f, FColor::Blue, false, -1.0f, 0, 2.0f);

				// Suspension force cyan arrow
				if (State.AppliedForce > 1.0f)
				{
					const FVector ForceVector = State.SurfaceNormal * (State.AppliedForce * ForceVectorScale);
					DrawDebugDirectionalArrow(World, MountPos, MountPos + ForceVector, 8.0f, FColor::Cyan, false, -1.0f, 0, 2.5f);
				}
			}
			else
			{
				// Red trace when airborne/no contact
				const FVector TraceEnd = MountPos + (LocalDown * Config.MaxTraceLength);
				DrawDebugLine(World, MountPos, TraceEnd, FColor::Red, false, -1.0f, 0, 1.5f);
			}
		}

		// Smoothed composite surface normal
		const FVector SmoothedNormal = RepulsorSystem->GetSmoothedSurfaceNormal();
		DrawDebugDirectionalArrow(World, VehicleLocation, VehicleLocation + (SmoothedNormal * 100.0f), 15.0f, FColor::Emerald, false, -1.0f, 0, 3.5f);
	}

	// Thrust Force vector (Orange)
	if (PhysicsComp->LastThrustApplied > 10.0f)
	{
		const FVector ThrustDir = VehicleTransform.GetUnitAxis(EAxis::X);
		const FVector ThrustVec = ThrustDir * (PhysicsComp->LastThrustApplied * ForceVectorScale);
		DrawDebugDirectionalArrow(World, VehicleLocation, VehicleLocation + ThrustVec, 12.0f, FColor::Orange, false, -1.0f, 0, 3.0f);
	}

	// Downforce vector (Magenta)
	if (PhysicsComp->LastDownforceApplied > 10.0f)
	{
		const FVector DownDir = -PhysicsComp->GetSmoothedSurfaceNormal();
		const FVector DownVec = DownDir * (PhysicsComp->LastDownforceApplied * ForceVectorScale);
		DrawDebugDirectionalArrow(World, VehicleLocation, VehicleLocation + DownVec, 12.0f, FColor::Magenta, false, -1.0f, 0, 3.0f);
	}
}

void UHoverBikeDebugComponent::DrawScreenHUD(const AHoverBike* Bike, const UHoverBikePhysicsComponent* PhysicsComp)
{
	if (!GEngine)
	{
		return;
	}

	const float LeanAngle = (PhysicsComp->GetStabilization()) ? PhysicsComp->GetStabilization()->GetCurrentLeanAngleDeg() : 0.0f;
	const float TargetLean = (PhysicsComp->GetStabilization()) ? PhysicsComp->GetStabilization()->GetTargetLeanAngleDeg() : 0.0f;
	const FVector AngularVel = Bike->GetBikeMesh()->GetPhysicsAngularVelocityInRadians();

	const FString Telemetry = HoverBikeDebug::FormatTelemetryString(
		PhysicsComp->GetCurrentSpeedKmh(),
		PhysicsComp->GetGroundedPadCount() >= 2,
		PhysicsComp->GetGroundedPadCount(),
		PhysicsComp->GetHoverHeight(),
		LeanAngle,
		TargetLean,
		PhysicsComp->GetCurrentDriftAlpha(),
		PhysicsComp->GetGripEnergyPercent(),
		PhysicsComp->LastThrustApplied,
		PhysicsComp->LastDownforceApplied,
		AngularVel,
		PhysicsComp->GetCurrentState()
	);

	GEngine->AddOnScreenDebugMessage(
		reinterpret_cast<uint64>(this),
		0.0f,
		FColor::Cyan,
		Telemetry,
		false,
		FVector2D(1.1f, 1.1f)
	);
}
