#include "Vehicle/RedlineMotorcyclePawn.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/SpotLightComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/StaticMeshComponent.h"

ARedlineMotorcyclePawn::ARedlineMotorcyclePawn()
{
	PrimaryActorTick.bCanEverTick = true;

	ChassisMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ChassisMesh"));
	ChassisMesh->SetSimulatePhysics(true);
	ChassisMesh->SetCollisionProfileName(TEXT("Vehicle"));
	RootComponent = ChassisMesh;

	PhysicsComponent = CreateDefaultSubobject<URedlineMotorcyclePhysicsComponent>(TEXT("PhysicsComponent"));

	// Third-Person Dynamic Chase Camera Rig
	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->SetupAttachment(RootComponent);
	SpringArm->TargetArmLength = 380.0f;
	SpringArm->SocketOffset = FVector(0.0f, 0.0f, 130.0f);
	SpringArm->bEnableCameraLag = true;
	SpringArm->CameraLagSpeed = 12.0f;
	SpringArm->bInheritRoll = false;

	ChaseCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ChaseCamera"));
	ChaseCamera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	// First-Person Cockpit / Helmet FPV Camera
	CockpitCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("CockpitCamera"));
	CockpitCamera->SetupAttachment(RootComponent);
	CockpitCamera->SetRelativeLocation(FVector(25.0f, 0.0f, 95.0f));
	CockpitCamera->bAutoActivate = false;

	// Lighting
	Headlight = CreateDefaultSubobject<USpotLightComponent>(TEXT("Headlight"));
	Headlight->SetupAttachment(RootComponent);
	Headlight->SetRelativeLocation(FVector(110.0f, 0.0f, 65.0f));
	Headlight->Intensity = 15000.0f;
	Headlight->OuterConeAngle = 38.0f;

	BrakeLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("BrakeLight"));
	BrakeLight->SetupAttachment(RootComponent);
	BrakeLight->SetRelativeLocation(FVector(-85.0f, 0.0f, 55.0f));
	BrakeLight->LightColor = FColor::Red;
	BrakeLight->Intensity = 2000.0f;
}

void ARedlineMotorcyclePawn::BeginPlay()
{
	Super::BeginPlay();

	if (PhysicsComponent)
	{
		PhysicsComponent->OnLowSideCrash.AddDynamic(this, &ARedlineMotorcyclePawn::HandleLowSideCrash);
	}
}

void ARedlineMotorcyclePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Nitro replenishment
	if (!bIsNitroActive)
	{
		CurrentNitroAmount = FMath::Min(100.0f, CurrentNitroAmount + (DeltaTime * 8.0f));
	}
	else
	{
		CurrentNitroAmount = FMath::Max(0.0f, CurrentNitroAmount - (DeltaTime * 25.0f));
		if (CurrentNitroAmount <= 0.0f)
		{
			SetNitroActive(false);
		}
	}

	// Camera FOV speed stretch
	if (ChaseCamera && PhysicsComponent)
	{
		const float SpeedKmh = PhysicsComponent->GetTelemetry().SpeedKmh;
		const float TargetFOV = FMath::Lerp(80.0f, 105.0f, FMath::Clamp(SpeedKmh / 280.0f, 0.0f, 1.0f));
		ChaseCamera->FieldOfView = FMath::FInterpTo(ChaseCamera->FieldOfView, TargetFOV, DeltaTime, 4.0f);
	}
}

void ARedlineMotorcyclePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &ARedlineMotorcyclePawn::InputThrottle);
	PlayerInputComponent->BindAxis("MoveRight", this, &ARedlineMotorcyclePawn::InputSteering);
	PlayerInputComponent->BindAxis("Brake", this, &ARedlineMotorcyclePawn::InputFrontBrake);

	PlayerInputComponent->BindAction("ShiftUp", IE_Pressed, this, &ARedlineMotorcyclePawn::InputShiftUp);
	PlayerInputComponent->BindAction("ShiftDown", IE_Pressed, this, &ARedlineMotorcyclePawn::InputShiftDown);
	PlayerInputComponent->BindAction("CycleCamera", IE_Pressed, this, &ARedlineMotorcyclePawn::CycleCameraMode);
	PlayerInputComponent->BindAction("ResetBike", IE_Pressed, this, &ARedlineMotorcyclePawn::RecoverMotorcycle);
}

void ARedlineMotorcyclePawn::InputThrottle(float Val)
{
	if (PhysicsComponent) PhysicsComponent->SetThrottleInput(Val);
}

void ARedlineMotorcyclePawn::InputSteering(float Val)
{
	if (PhysicsComponent) PhysicsComponent->SetSteeringInput(Val);
}

void ARedlineMotorcyclePawn::InputFrontBrake(float Val)
{
	if (PhysicsComponent)
	{
		PhysicsComponent->SetFrontBrakeInput(Val);
		if (BrakeLight)
		{
			BrakeLight->SetIntensity(Val > 0.1f ? 12000.0f : 2000.0f);
		}
	}
}

void ARedlineMotorcyclePawn::InputRearBrake(float Val)
{
	if (PhysicsComponent) PhysicsComponent->SetRearBrakeInput(Val);
}

void ARedlineMotorcyclePawn::InputShiftUp()
{
	if (PhysicsComponent) PhysicsComponent->ShiftUp();
}

void ARedlineMotorcyclePawn::InputShiftDown()
{
	if (PhysicsComponent) PhysicsComponent->ShiftDown();
}

void ARedlineMotorcyclePawn::CycleCameraMode()
{
	CurrentCameraMode = static_cast<ERedlineCameraMode>((static_cast<uint8>(CurrentCameraMode) + 1) % 4);

	if (ChaseCamera && CockpitCamera)
	{
		switch (CurrentCameraMode)
		{
		case ERedlineCameraMode::Chase:
			ChaseCamera->SetActive(true);
			CockpitCamera->SetActive(false);
			SpringArm->TargetArmLength = 380.0f;
			break;
		case ERedlineCameraMode::CockpitFPV:
			ChaseCamera->SetActive(false);
			CockpitCamera->SetActive(true);
			break;
		case ERedlineCameraMode::KneeDown:
			ChaseCamera->SetActive(true);
			CockpitCamera->SetActive(false);
			SpringArm->TargetArmLength = 220.0f;
			SpringArm->SocketOffset = FVector(0.0f, 60.0f, 40.0f);
			break;
		case ERedlineCameraMode::Cinematic:
			ChaseCamera->SetActive(true);
			CockpitCamera->SetActive(false);
			SpringArm->TargetArmLength = 550.0f;
			SpringArm->SocketOffset = FVector(0.0f, 0.0f, 220.0f);
			break;
		}
	}
}

void ARedlineMotorcyclePawn::SetNitroActive(bool bActive)
{
	bIsNitroActive = bActive && (CurrentNitroAmount > 10.0f);
}

void ARedlineMotorcyclePawn::TriggerHorn()
{
	// Trigger horn event or audio cue
}

void ARedlineMotorcyclePawn::RecoverMotorcycle()
{
	FRotator CurrentRot = GetActorRotation();
	CurrentRot.Roll = 0.0f;
	CurrentRot.Pitch = 0.0f;
	SetActorRotation(CurrentRot);

	if (ChassisMesh)
	{
		ChassisMesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
		ChassisMesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
	}

	if (PhysicsComponent)
	{
		PhysicsComponent->ResetBikeState();
	}
}

void ARedlineMotorcyclePawn::HandleLowSideCrash()
{
	// Ragdoll rider and slide bike
}
