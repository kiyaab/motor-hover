// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BoostPadActor.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnBoostPadTriggeredSignature, AActor*, RacerActor);

/**
 * High-energy magnetic accelerator surface pad.
 * Injects a forward physical impulse to the vehicle's rigid body and triggers plasma visual effects.
 */
UCLASS()
class HOVERRACER_API ABoostPadActor : public AActor
{
	GENERATED_BODY()

public:
	ABoostPadActor();

	virtual void BeginPlay() override;

	/** Forward velocity impulse magnitude added to the vehicle in cm/s */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Boost", meta = (ClampMin = "500.0", ClampMax = "10000.0"))
	float BoostVelocityImpulse = 3500.0f;

	/** Duration to temporarily multiply forward thruster force (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Boost")
	float BoostThrustMultiplierDuration = 1.2f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UBoxComponent> TriggerBox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	TObjectPtr<UStaticMeshComponent> PadMesh;

	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnBoostPadTriggeredSignature OnBoostPadTriggered;

	UFUNCTION(BlueprintNativeEvent, Category = "Race|VFX")
	void TriggerPadVFX();

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
