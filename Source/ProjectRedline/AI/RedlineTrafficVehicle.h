#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AI/RedlineTrafficTypes.h"
#include "RedlineTrafficVehicle.generated.h"

class UStaticMeshComponent;
class UPointLightComponent;

UCLASS()
class PROJECTREDLINE_API ARedlineTrafficVehicle : public AActor
{
	GENERATED_BODY()
	
public:	
	ARedlineTrafficVehicle();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* VehicleMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
	UPointLightComponent* LeftBrakeLight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
	UPointLightComponent* RightBrakeLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Config")
	FRedlineTrafficVehicleConfig Config;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic AI")
	float LaneOffsetCm = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Traffic AI")
	float CurrentSpeedKmh = 80.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	bool bHasBeenOvertaken = false;

	UFUNCTION(BlueprintCallable, Category = "Traffic AI")
	void ReactToHorn();

protected:
	virtual void BeginPlay() override;

private:
	bool bIsBraking = false;
};
