// Copyright Epic Games, Inc. All Rights Reserved.

#include "FinishLineActor.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

AFinishLineActor::AFinishLineActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));

	ArchwayMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ArchwayMesh"));
	ArchwayMesh->SetupAttachment(RootComponent);

	HolographicBannerMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HolographicBannerMesh"));
	HolographicBannerMesh->SetupAttachment(RootComponent);
	HolographicBannerMesh->SetCollisionProfileName(TEXT("NoCollision"));

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	TriggerBox->SetupAttachment(RootComponent);
	TriggerBox->SetBoxExtent(FVector(150.0f, 2500.0f, 1500.0f));
	TriggerBox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	TriggerBox->SetGenerateOverlapEvents(true);
}

void AFinishLineActor::BeginPlay()
{
	Super::BeginPlay();

	if (TriggerBox)
	{
		TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &AFinishLineActor::OnOverlapBegin);
	}
}

void AFinishLineActor::OnOverlapBegin(
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
	OnFinishLineCrossed.Broadcast(OtherActor, CurrentTime);
	TriggerFinishCelebration();
}

void AFinishLineActor::TriggerFinishCelebration_Implementation()
{
	// Holographic fireworks and sound effects
}
