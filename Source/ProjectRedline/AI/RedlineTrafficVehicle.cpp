#include "AI/RedlineTrafficVehicle.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"

ARedlineTrafficVehicle::ARedlineTrafficVehicle()
{
	PrimaryActorTick.bCanEverTick = true;

	VehicleMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VehicleMesh"));
	VehicleMesh->SetCollisionProfileName(TEXT("Vehicle"));
	RootComponent = VehicleMesh;

	LeftBrakeLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("LeftBrakeLight"));
	LeftBrakeLight->SetupAttachment(RootComponent);
	LeftBrakeLight->SetRelativeLocation(FVector(-210.0f, -75.0f, 65.0f));
	LeftBrakeLight->LightColor = FColor::Red;
	LeftBrakeLight->Intensity = 1500.0f;

	RightBrakeLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("RightBrakeLight"));
	RightBrakeLight->SetupAttachment(RootComponent);
	RightBrakeLight->SetRelativeLocation(FVector(-210.0f, 75.0f, 65.0f));
	RightBrakeLight->LightColor = FColor::Red;
	RightBrakeLight->Intensity = 1500.0f;
}

void ARedlineTrafficVehicle::BeginPlay()
{
	Super::BeginPlay();
	CurrentSpeedKmh = Config.DesiredSpeedKmh;
}

void ARedlineTrafficVehicle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Forward motion along actor heading
	const float SpeedCmS = CurrentSpeedKmh * (100000.0f / 3600.0f);
	const FVector DeltaLocation = GetActorForwardVector() * (SpeedCmS * DeltaTime);
	AddActorWorldOffset(DeltaLocation, true);

	// Update brake lights
	const float LightIntensity = bIsBraking ? 9000.0f : 1500.0f;
	if (LeftBrakeLight) LeftBrakeLight->SetIntensity(LightIntensity);
	if (RightBrakeLight) RightBrakeLight->SetIntensity(LightIntensity);
}

void ARedlineTrafficVehicle::ReactToHorn()
{
	// Swerve slightly away or honk back
	LaneOffsetCm += (LaneOffsetCm > 0.0f ? 80.0f : -80.0f);
}
