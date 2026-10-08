// Copyright Epic Games, Inc. All Rights Reserved.

#include "BoostPadActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Vehicle/HoverBike.h"
#include "Vehicle/HoverBikePhysicsComponent.h"

ABoostPadActor::ABoostPadActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));

	PadMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PadMesh"));
	PadMesh->SetupAttachment(RootComponent);
	PadMesh->SetCollisionProfileName(TEXT("BlockAll"));

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetBoxExtent(FVector(300.0f, 600.0f, 150.0f));
	TriggerBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerBox->SetGenerateOverlapEvents(true);
}

void ABoostPadActor::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ABoostPadActor::OnOverlapBegin);
	}
}

void ABoostPadActor::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor)
	{
		return;
	}

	AHoverBike* Bike = Cast<AHoverBike>(OtherActor);
	if (Bike)
	{
		UStaticMeshComponent* Mesh = Bike->GetBikeMesh();
		if (Mesh && Mesh->IsSimulatingPhysics())
		{
			// Add forward impulse along vehicle's heading
			const FVector Forward = Mesh->GetForwardVector();
			Mesh->AddImpulse(Forward * (BoostVelocityImpulse * Mesh->GetMass()), NAME_None, false);
		}

		OnBoostPadTriggered.Broadcast(Bike);
		TriggerPadVFX();
	}
}

void ABoostPadActor::TriggerPadVFX_Implementation()
{
	// Visual orange plasma flash and audio trigger
}
