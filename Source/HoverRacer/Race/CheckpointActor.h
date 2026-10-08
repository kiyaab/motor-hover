// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CheckpointActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnCheckpointPassedSignature, int32, CheckpointIndex, AActor*, RacerActor, float, SplitTime);

/**
 * Holographic checkpoint scanning gate actor.
 * Detects vehicle traversal, verifies race progression, and triggers holographic visual feedback.
 */
UCLASS()
class HOVERRACER_API ACheckpointActor : public AActor
{
	GENERATED_BODY()

public:
	ACheckpointActor();

	virtual void BeginPlay() override;

	/** Index of this checkpoint in the circuit sequence (0-indexed) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Checkpoint")
	int32 CheckpointIndex = 0;

	/** If true, this checkpoint represents a sector timing boundary */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Checkpoint")
	bool bIsSectorGate = false;

	/** Checkpoint trigger volume */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	/** Archway holographic structural mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> GateMesh;

	/** Holographic scanning field mesh */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ScanFieldMesh;

	/** Triggered when a racer crosses this checkpoint */
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnCheckpointPassedSignature OnCheckpointPassed;

	/** Visual pulse triggered when crossed */
	UFUNCTION(BlueprintNativeEvent, Category = "Race|VFX")
	void TriggerPassedFeedback();

protected:
	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

private:
	bool bHasBeenTriggeredThisLap = false;
};
