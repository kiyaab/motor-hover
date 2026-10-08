// Copyright Epic Games, Inc. All Rights Reserved.

#include "TrackSplineActor.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"

ATrackSplineActor::ATrackSplineActor()
{
	PrimaryActorTick.bCanEverTick = false;

	TrackSpline = CreateDefaultSubobject<USplineComponent>(TEXT("TrackSpline"));
	SetRootComponent(TrackSpline);
	TrackSpline->SetClosedLoop(false);
}

void ATrackSplineActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	RebuildTrackGeometry();
}

void ATrackSplineActor::RebuildTrackGeometry()
{
	for (USplineMeshComponent* MeshComp : SpawnedSplineMeshes)
	{
		if (MeshComp)
		{
			MeshComp->DestroyComponent();
		}
	}
	SpawnedSplineMeshes.Empty();

	if (!TrackSpline || !TrackSegmentMesh)
	{
		return;
	}

	const float SplineLength = TrackSpline->GetSplineLength();
	const int32 NumSegments = FMath::Max(1, FMath::CeilToInt(SplineLength / SegmentLength));

	for (int32 i = 0; i < NumSegments; ++i)
	{
		const float StartDist = i * SegmentLength;
		const float EndDist = FMath::Min(SplineLength, (i + 1) * SegmentLength);

		const FVector StartPos = TrackSpline->GetLocationAtDistanceAlongSpline(StartDist, ESplineCoordinateSpace::Local);
		const FVector StartTangent = TrackSpline->GetTangentAtDistanceAlongSpline(StartDist, ESplineCoordinateSpace::Local);
		const FVector EndPos = TrackSpline->GetLocationAtDistanceAlongSpline(EndDist, ESplineCoordinateSpace::Local);
		const FVector EndTangent = TrackSpline->GetTangentAtDistanceAlongSpline(EndDist, ESplineCoordinateSpace::Local);

		USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
		if (SplineMesh)
		{
			SplineMesh->SetStaticMesh(TrackSegmentMesh);
			SplineMesh->SetMobility(EComponentMobility::Movable);
			SplineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
			SplineMesh->AttachToComponent(TrackSpline, FAttachmentTransformRules::KeepRelativeTransform);
			SplineMesh->SetStartAndEnd(StartPos, StartTangent, EndPos, EndTangent, true);
			SplineMesh->SetCollisionProfileName(TEXT("BlockAll"));
			SplineMesh->RegisterComponent();
			SpawnedSplineMeshes.Add(SplineMesh);
		}
	}
}
