#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Vehicle/RedlineMotorcyclePhysicsComponent.h"
#include "RedlineMotorcyclePawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class USpotLightComponent;
class UPointLightComponent;
class UAudioComponent;

UENUM(BlueprintType)
enum class ERedlineCameraMode : uint8
{
	Chase      UMETA(DisplayName = "Chase Camera (Third-Person)"),
	CockpitFPV UMETA(DisplayName = "Cockpit / Helmet FPV"),
	KneeDown   UMETA(DisplayName = "Knee-Down Onboard"),
	Cinematic  UMETA(DisplayName = "Cinematic / Drone")
};

UCLASS()
class PROJECTREDLINE_API ARedlineMotorcyclePawn : public APawn
{
	GENERATED_BODY()

public:
	ARedlineMotorcyclePawn();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "Camera")
	void CycleCameraMode();

	UFUNCTION(BlueprintCallable, Category = "Performance")
	void SetNitroActive(bool bActive);

	UFUNCTION(BlueprintCallable, Category = "Audio")
	void TriggerHorn();

	UFUNCTION(BlueprintCallable, Category = "State")
	void RecoverMotorcycle();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* ChassisMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	URedlineMotorcyclePhysicsComponent* PhysicsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	USpringArmComponent* SpringArm;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* ChaseCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	UCameraComponent* CockpitCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
	USpotLightComponent* Headlight;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lighting")
	UPointLightComponent* BrakeLight;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	ERedlineCameraMode CurrentCameraMode = ERedlineCameraMode::Chase;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Nitro")
	float NitroDurationSeconds = 6.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Nitro")
	float CurrentNitroAmount = 100.0f;

protected:
	virtual void BeginPlay() override;

	void InputThrottle(float Val);
	void InputSteering(float Val);
	void InputFrontBrake(float Val);
	void InputRearBrake(float Val);
	void InputShiftUp();
	void InputShiftDown();

	UFUNCTION()
	void HandleLowSideCrash();

private:
	bool bIsNitroActive = false;
};
