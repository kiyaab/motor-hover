// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HoverBikeDebugComponent.generated.h"

class AHoverBike;
class UHoverBikePhysicsComponent;

/**
 * 3D visual debugger and 2D canvas telemetry HUD for the hoverbike.
 * Draws repulsor sweeps, contact normals, force vectors, and performance metrics.
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class HOVERRACER_API UHoverBikeDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UHoverBikeDebugComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	/** Toggle 3D force vectors and trace visualization */
	UFUNCTION(BlueprintCallable, Category = "HoverBike|Debug")
	void ToggleDebugVisuals() { bShowDebugVisuals = !bShowDebugVisuals; }

	/** Toggle on-screen telemetry text HUD */
	UFUNCTION(BlueprintCallable, Category = "HoverBike|Debug")
	void ToggleDebugHUD() { bShowDebugHUD = !bShowDebugHUD; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HoverBike|Debug")
	bool bShowDebugVisuals = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HoverBike|Debug")
	bool bShowDebugHUD = true;

	/** Vector draw scale (cm per Newton) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HoverBike|Debug")
	float ForceVectorScale = 0.001f;

private:
	void Draw3DVisuals(const AHoverBike* Bike, const UHoverBikePhysicsComponent* PhysicsComp);
	void DrawScreenHUD(const AHoverBike* Bike, const UHoverBikePhysicsComponent* PhysicsComp);
};
