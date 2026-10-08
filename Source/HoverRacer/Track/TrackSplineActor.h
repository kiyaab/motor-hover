// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TrackSplineActor.generated.h"

class USplineComponent;
class USplineMeshComponent;

UENUM(BlueprintType)
enum class ETrackSectionType : uint8
{
	FlatRaceway       UMETA(DisplayName = "Flat Raceway"),
	BankedTurn        UMETA(DisplayName = "Banked Turn"),
	VerticalWallRide  UMETA(DisplayName = "Vertical Wall Ride"),
	LoopTheLoop       UMETA(DisplayName = "Loop The Loop"),
	CanyonJumpRamp    UMETA(DisplayName = "Canyon Jump Ramp"),
	NeonTunnel        UMETA(DisplayName = "Neon Tunnel"),
	FloatingPlatform  UMETA(DisplayName = "Floating Platform")
};

/**
 * Procedural spline-based anti-gravity track generator actor.
 * Automatically deforms track segment meshes along custom 3D track curves.
 */
UCLASS()
class HOVERRACER_API ATrackSplineActor : public AActor
{
	GENERATED_BODY()

public:
	ATrackSplineActor();

	virtual void OnConstruction(const FTransform& Transform) override;

	/** Spline defining the 3D track path */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Track|Spline")
	TObjectPtr<USplineComponent> TrackSpline;

	/** Track surface static mesh to tile along spline */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track|Mesh")
	TObjectPtr<UStaticMesh> TrackSegmentMesh;

	/** Glowing border energy rail mesh */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track|Mesh")
	TObjectPtr<UStaticMesh> EnergyRailMesh;

	/** Width of the drivable racing ribbon (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track|Dimensions", meta = (ClampMin = "500.0", ClampMax = "8000.0"))
	float TrackWidth = 3600.0f;

	/** Length of each mesh slice along the spline (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track|Dimensions")
	float SegmentLength = 1000.0f;

	/** Track section classification */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Track|Classification")
	ETrackSectionType SectionType = ETrackSectionType::FlatRaceway;

	/** Procedurally build spline meshes */
	UFUNCTION(BlueprintCallable, Category = "Track|Generation")
	void RebuildTrackGeometry();

private:
	UPROPERTY()
	TArray<TObjectPtr<USplineMeshComponent>> SpawnedSplineMeshes;
};
