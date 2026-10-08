// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FinishLineActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFinishLineCrossedSignature, AActor*, RacerActor, float, LapTime);

/**
 * Flagship holographic starting/finish archway.
 * Detects lap completion, triggers checkered energy gates and fireworks/VFX.
 */
UCLASS()
class HOVERRACER_API AFinishLineActor : public AActor
{
	GENERATED_BODY()

public:
	AFinishLineActor();

	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> ArchwayMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> HolographicBannerMesh;

	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnFinishLineCrossedSignature OnFinishLineCrossed;

	UFUNCTION(BlueprintNativeEvent, Category = "Race|VFX")
	void TriggerFinishCelebration();

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
};
