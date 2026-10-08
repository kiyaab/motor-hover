// Copyright Epic Games, Inc. All Rights Reserved.

#include "MotorcyclePawn.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "MotorcyclePhysicsComponent.h"
#include "MotorcycleEngineComponent.h"
#include "MotorcycleTireComponent.h"

AMotorcyclePawn::AMotorcyclePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	// 1. Chassis Rigid Body (195 kg Superbike)
	ChassisMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChassisMesh"));
	SetRootComponent(ChassisMesh);
	ChassisMesh->SetSimulatePhysics(true);
	ChassisMesh->SetEnableGravity(true);
	ChassisMesh->SetCollisionProfileName(UCollisionProfile::Vehicle_ProfileName);
	ChassisMesh->SetMassOverrideInKg(NAME_None, 195.0f, true);
	ChassisMesh->SetLinearDamping(0.02f);
	ChassisMesh->SetAngularDamping(0.15f);

	// 2. Front Wheel & Fork (Wheelbase = 144 cm, Front axle at +72 cm)
	FrontWheel = CreateDefaultSubobject<UMotorcycleTireComponent>(TEXT("FrontWheel"));
	FrontWheel->SetupAttachment(ChassisMesh);
	FrontWheel->SetRelativeLocation(FVector(72.0f, 0.0f, -25.0f));
	FrontWheel->bIsFrontWheel = true;
	FrontWheel->SuspensionConfig.SpringStiffness = 38000.0f;
	FrontWheel->SuspensionConfig.DampingCoefficient = 4200.0f;
	FrontWheel->SuspensionConfig.MaxTravel = 12.0f;

	// 3. Rear Wheel & Swingarm (Rear axle at -72 cm)
	RearWheel = CreateDefaultSubobject<UMotorcycleTireComponent>(TEXT("RearWheel"));
	RearWheel->SetupAttachment(ChassisMesh);
	RearWheel->SetRelativeLocation(FVector(-72.0f, 0.0f, -25.0f));
	RearWheel->bIsFrontWheel = false;
	RearWheel->SuspensionConfig.SpringStiffness = 44000.0f;
	RearWheel->SuspensionConfig.DampingCoefficient = 4800.0f;
	RearWheel->SuspensionConfig.MaxTravel = 13.0f;

	// 4. Engine & 6-Speed Transmission
	EngineComponent = CreateDefaultSubobject<UMotorcycleEngineComponent>(TEXT("EngineComponent"));

	// 5. 2-Wheel Physics Solver
	PhysicsComponent = CreateDefaultSubobject<UMotorcyclePhysicsComponent>(TEXT("PhysicsComponent"));

	// 6. Camera Boom & Chase Camera
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(ChassisMesh);
	CameraBoom->TargetArmLength = 340.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 85.0f);
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritPitch = true;
	CameraBoom->bInheritYaw = true;
	CameraBoom->bInheritRoll = false; // Smooth camera tilt during high-angle lean
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 15.0f;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->FieldOfView = 90.0f;
}

void AMotorcyclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AMotorcyclePawn::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateCameraDynamics(DeltaSeconds);
}

void AMotorcyclePawn::UpdateCameraDynamics(float DeltaSeconds)
{
	if (!Camera || !PhysicsComponent)
	{
		return;
	}

	const float SpeedKmh = PhysicsComponent->GetSpeedKmh();
	const float TargetFOV = FMath::Lerp(88.0f, 108.0f, FMath::Clamp(SpeedKmh / 280.0f, 0.0f, 1.0f));
	Camera->SetFieldOfView(FMath::FInterpTo(Camera->FieldOfView, TargetFOV, DeltaSeconds, 4.0f));

	// Subtle camera tilt following motorcycle lean (absorbing ~30% of bike roll)
	const float LeanDeg = PhysicsComponent->GetCurrentLeanAngleDeg();
	Camera->SetRelativeRotation(FRotator(0.0f, 0.0f, LeanDeg * 0.32f));
}

void AMotorcyclePawn::SetThrottle(float Value)
{
	if (PhysicsComponent)
	{
		PhysicsComponent->SetThrottleInput(Value);
	}
}

void AMotorcyclePawn::SetFrontBrake(float Value)
{
	if (PhysicsComponent)
	{
		PhysicsComponent->SetFrontBrakeInput(Value);
	}
}

void AMotorcyclePawn::SetRearBrake(float Value)
{
	if (PhysicsComponent)
	{
		PhysicsComponent->SetRearBrakeInput(Value);
	}
}

void AMotorcyclePawn::SetSteer(float Value)
{
	if (PhysicsComponent)
	{
		PhysicsComponent->SetSteeringInput(Value);
	}
}

void AMotorcyclePawn::ShiftUp()
{
	if (EngineComponent)
	{
		EngineComponent->ShiftUp();
	}
}

void AMotorcyclePawn::ShiftDown()
{
	if (EngineComponent)
	{
		EngineComponent->ShiftDown();
	}
}

void AMotorcyclePawn::ResetBike()
{
	if (ChassisMesh)
	{
		ChassisMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
		ChassisMesh->SetPhysicsAngularVelocityInRadians(FVector::ZeroVector);
		const FVector CurrentLoc = GetActorLocation();
		SetActorLocationAndRotation(FVector(CurrentLoc.X, CurrentLoc.Y, CurrentLoc.Z + 40.0f), FRotator(0.0f, GetActorRotation().Yaw, 0.0f), false, nullptr, ETeleportType::TeleportPhysics);
	}
}
