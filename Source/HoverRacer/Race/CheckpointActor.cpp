// Copyright Epic Games, Inc. All Rights Reserved.

#include "CheckpointActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

ACheckpointActor::ACheckpointActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));

	GateMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GateMesh"));
	GateMesh->SetupAttachment(RootComponent);
	GateMesh->SetCollisionProfileName(TEXT("NoCollision"));

	ScanFieldMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ScanFieldMesh"));
	ScanFieldMesh->SetupAttachment(RootComponent);
	ScanFieldMesh->SetCollisionProfileName(TEXT("NoCollision"));

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetBoxExtent(FVector(150.0f, 2000.0f, 1200.0f));
	TriggerBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerBox->SetGenerateOverlapEvents(true);
}

void ACheckpointActor::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ACheckpointActor::OnOverlapBegin);
	}
}

void ACheckpointActor::OnOverlapBegin(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (!OtherActor || OtherActor == this)
	{
		return;
	}

	const float CurrentTime = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.0f;
	OnCheckpointPassed.Broadcast(CheckpointIndex, OtherActor, CurrentTime);
	TriggerPassedFeedback();
}

void ACheckpointActor::TriggerPassedFeedback_Implementation()
{
	// Blueprint hook for holographic scan wave and audio chime
}
