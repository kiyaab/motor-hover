// Copyright Epic Games, Inc. All Rights Reserved.

#include "RiderIKComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Motorcycle/MotorcyclePawn.h"
#include "Motorcycle/MotorcyclePhysicsComponent.h"

URiderIKComponent::URiderIKComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.TickGroup = TG_PostPhysics;
}

void URiderIKComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsRagdoll)
	{
		return;
	}

	AMotorcyclePawn* Bike = Cast<AMotorcyclePawn>(GetOwner());
	if (!Bike)
	{
		return;
	}

	UMotorcyclePhysicsComponent* Physics = Bike->GetPhysicsComponent();
	if (!Physics)
	{
		return;
	}

	const float SpeedKmh = Physics->GetSpeedKmh();
	const float LeanDeg = Physics->GetCurrentLeanAngleDeg();

	// 1. Aerodynamic tuck at high speeds (> 160 km/h)
	bIsTuckedIn = (SpeedKmh > 160.0f);

	// 2. Rider Spine & Body Hang-Off (shifting inside of corner)
	const float TargetSpineOffset = FMath::Clamp(LeanDeg / 58.0f, -1.0f, 1.0f);
	SpineLeanOffset = FMath::FInterpTo(SpineLeanOffset, TargetSpineOffset, DeltaTime, 8.0f);

	// 3. Knee-Down Cornering Extension (active when leaned past 35 degrees)
	const float AbsLean = FMath::Abs(LeanDeg);
	if (AbsLean > 35.0f)
	{
		const float TargetKnee = (AbsLean - 35.0f) / (58.0f - 35.0f);
		KneeExtension = FMath::FInterpTo(KneeExtension, FMath::Clamp(TargetKnee, 0.0f, 1.0f), DeltaTime, 10.0f);
	}
	else
	{
		KneeExtension = FMath::FInterpTo(KneeExtension, 0.0f, DeltaTime, 8.0f);
	}

	// 4. Trigger ragdoll if motorcycle crashes
	if (Physics->GetCrashState() != EMotorcycleCrashType::None)
	{
		TriggerRiderRagdoll();
	}
}

void URiderIKComponent::TriggerRiderRagdoll()
{
	if (bIsRagdoll)
	{
		return;
	}

	bIsRagdoll = true;

	if (RiderMesh)
	{
		RiderMesh->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		RiderMesh->SetCollisionProfileName(UCollisionProfile::PhysicsActor_ProfileName);
		RiderMesh->SetSimulatePhysics(true);

		// Carry over parent motorcycle velocity to ragdoll body
		AActor* Owner = GetOwner();
		if (Owner)
		{
			UPrimitiveComponent* BikeRoot = Cast<UPrimitiveComponent>(Owner->GetRootComponent());
			if (BikeRoot)
			{
				RiderMesh->SetPhysicsLinearVelocity(BikeRoot->GetPhysicsLinearVelocity());
			}
		}
	}
}
