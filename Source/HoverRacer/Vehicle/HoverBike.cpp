// Copyright Epic Games, Inc. All Rights Reserved.

#include "HoverBike.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Vehicle/HoverBikePhysicsComponent.h"
#include "Vehicle/HoverBikeInputComponent.h"
#include "Debug/HoverBikeDebugComponent.h"
#include "Data/HoverBikeTuningData.h"

AHoverBike::AHoverBike()
{
	PrimaryActorTick.bCanEverTick = true;

	// Root Chaos Rigid Body
	BikeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BikeMesh"));
	SetRootComponent(BikeMesh);
	BikeMesh->SetSimulatePhysics(true);
	BikeMesh->SetEnableGravity(true);
	BikeMesh->SetCollisionProfileName(UCollisionProfile::Vehicle_ProfileName);
	BikeMesh->SetLinearDamping(0.01f);
	BikeMesh->SetAngularDamping(0.1f);
	BikeMesh->SetMassOverrideInKg(NAME_None, 350.0f, true);

	// Physics Simulation Component
	PhysicsComponent = CreateDefaultSubobject<UHoverBikePhysicsComponent>(TEXT("PhysicsComponent"));

	// Input Management Component
	InputComponentInternal = CreateDefaultSubobject<UHoverBikeInputComponent>(TEXT("InputComponentInternal"));

	// Dynamic Racing Camera Boom
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(BikeMesh);
	CameraBoom->TargetArmLength = 480.0f;
	CameraBoom->SocketOffset = FVector(0.0f, 0.0f, 140.0f);
	CameraBoom->bUsePawnControlRotation = false;
	CameraBoom->bInheritPitch = true;
	CameraBoom->bInheritYaw = true;
	CameraBoom->bInheritRoll = false; // Smooth camera tilt rather than hard vehicle roll
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->bEnableCameraRotationLag = true;
	CameraBoom->CameraLagSpeed = 12.0f;
	CameraBoom->CameraRotationLagSpeed = 14.0f;
	CameraBoom->CameraLagMaxDistance = 150.0f;

	// Follow Camera
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	Camera->bUsePawnControlRotation = false;
	Camera->FieldOfView = 90.0f;

	// Debug HUD & 3D Visualizer Component
	DebugComponent = CreateDefaultSubobject<UHoverBikeDebugComponent>(TEXT("DebugComponent"));
}

void AHoverBike::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (InputComponentInternal)
	{
		InputComponentInternal->OnRecoveryRequested.AddDynamic(this, &AHoverBike::TriggerRecovery);
	}

	if (PhysicsComponent && PhysicsComponent->GetRepulsorSystem())
	{
		PhysicsComponent->GetRepulsorSystem()->OnRepulsorContact.AddDynamic(this, &AHoverBike::OnRepulsorContact);
		PhysicsComponent->GetRepulsorSystem()->OnRepulsorLost.AddDynamic(this, &AHoverBike::OnRepulsorLost);
	}

	if (PhysicsComponent && PhysicsComponent->GetDriftSystem())
	{
		PhysicsComponent->GetDriftSystem()->OnDriftStarted.AddDynamic(this, &AHoverBike::OnDriftStarted);
		PhysicsComponent->GetDriftSystem()->OnDriftEnded.AddDynamic(this, &AHoverBike::OnDriftEnded);
	}
}

void AHoverBike::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EnhancedInputComp = Cast<UEnhancedInputComponent>(PlayerInputComponent);
	if (EnhancedInputComp && InputConfig)
	{
		if (APlayerController* PC = Cast<APlayerController>(GetController()))
		{
			if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
			{
				if (InputConfig->DefaultMappingContext)
				{
					Subsystem->AddMappingContext(InputConfig->DefaultMappingContext, InputConfig->MappingPriority);
				}
			}
		}

		if (InputComponentInternal)
		{
			InputComponentInternal->SetupPlayerInput(EnhancedInputComp, InputConfig);
		}
	}
}

void AHoverBike::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateCameraDynamics(DeltaSeconds);
}

void AHoverBike::UpdateCameraDynamics(float DeltaSeconds)
{
	if (!Camera || !PhysicsComponent || !PhysicsComponent->TuningData)
	{
		return;
	}

	const UHoverBikeTuningData* Tuning = PhysicsComponent->TuningData;
	const float SpeedNormalized = GetSpeedNormalized();

	// Dynamic FOV scaling with speed
	const float TargetFOV = FMath::Lerp(Tuning->BaseFOV, Tuning->MaxSpeedFOV, SpeedNormalized);
	CurrentCameraFOV = FMath::FInterpTo(CurrentCameraFOV, TargetFOV, DeltaSeconds, 4.0f);
	Camera->SetFieldOfView(CurrentCameraFOV);

	// Subtle camera tilt with motorcycle lean
	if (PhysicsComponent->GetStabilization())
	{
		const float LeanDeg = PhysicsComponent->GetStabilization()->GetCurrentLeanAngleDeg();
		const float SubtleTilt = LeanDeg * 0.25f; // 25% camera tilt absorption
		Camera->SetRelativeRotation(FRotator(0.0f, 0.0f, SubtleTilt));
	}
}

// AI Control Interface
void AHoverBike::SetThrottleInput(float Value)
{
	if (InputComponentInternal)
	{
		InputComponentInternal->SetThrottleInput(Value);
	}
}

void AHoverBike::SetSteeringInput(float Value)
{
	if (InputComponentInternal)
	{
		InputComponentInternal->SetSteeringInput(Value);
	}
}

void AHoverBike::SetBrakeInput(float Value)
{
	if (InputComponentInternal)
	{
		InputComponentInternal->SetBrakeInput(Value);
	}
}

void AHoverBike::SetDriftInput(bool bActive)
{
	if (InputComponentInternal)
	{
		InputComponentInternal->SetDriftInput(bActive);
	}
}

void AHoverBike::SetBoostInput(bool bActive)
{
	if (InputComponentInternal)
	{
		InputComponentInternal->SetBoostInput(bActive);
	}
}

void AHoverBike::SetAirBrakeInput(float Value)
{
	if (InputComponentInternal)
	{
		InputComponentInternal->SetAirBrakeInput(Value);
	}
}

void AHoverBike::TriggerRecovery()
{
	if (PhysicsComponent)
	{
		PhysicsComponent->TriggerRecovery();
	}
}

// Audio & Visual Telemetry Hooks
float AHoverBike::GetSpeedNormalized() const
{
	if (!PhysicsComponent || !PhysicsComponent->TuningData)
	{
		return 0.0f;
	}
	return FMath::Clamp(PhysicsComponent->GetCurrentSpeedKmh() / PhysicsComponent->TuningData->MaxSpeedKmh, 0.0f, 1.5f);
}

float AHoverBike::GetThrottleAmount() const
{
	return InputComponentInternal ? InputComponentInternal->GetThrottleInput() : 0.0f;
}

float AHoverBike::GetBoostAmount() const
{
	return PhysicsComponent ? (PhysicsComponent->GetBoostEnergyPercent() * 0.01f) : 0.0f;
}

float AHoverBike::GetDriftAmount() const
{
	return PhysicsComponent ? PhysicsComponent->GetCurrentDriftAlpha() : 0.0f;
}

float AHoverBike::GetRepulsorLoad() const
{
	if (!PhysicsComponent || !PhysicsComponent->GetRepulsorSystem())
	{
		return 0.0f;
	}
	float TotalForce = 0.0f;
	const TArray<FRepulsorPadState>& States = PhysicsComponent->GetRepulsorSystem()->GetPadStates();
	for (const FRepulsorPadState& State : States)
	{
		TotalForce += State.AppliedForce;
	}
	return FMath::Clamp(TotalForce / 300000.0f, 0.0f, 4.0f);
}

float AHoverBike::GetAirborneAmount() const
{
	if (!PhysicsComponent)
	{
		return 0.0f;
	}
	return (PhysicsComponent->GetCurrentState() == EHoverBikeState::Airborne) ? 1.0f : 0.0f;
}
