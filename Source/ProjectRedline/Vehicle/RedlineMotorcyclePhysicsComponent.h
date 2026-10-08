#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RedlineMotorcycleTypes.h"
#include "RedlineMotorcyclePhysicsComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRedlineGearChanged, int32, NewGear);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRedlineLowSideCrash);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTREDLINE_API URedlineMotorcyclePhysicsComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URedlineMotorcyclePhysicsComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Controls")
	void SetThrottleInput(float Val) { ThrottleInput = FMath::Clamp(Val, 0.0f, 1.0f); }

	UFUNCTION(BlueprintCallable, Category = "Controls")
	void SetFrontBrakeInput(float Val) { FrontBrakeInput = FMath::Clamp(Val, 0.0f, 1.0f); }

	UFUNCTION(BlueprintCallable, Category = "Controls")
	void SetRearBrakeInput(float Val) { RearBrakeInput = FMath::Clamp(Val, 0.0f, 1.0f); }

	UFUNCTION(BlueprintCallable, Category = "Controls")
	void SetSteeringInput(float Val) { SteeringInput = FMath::Clamp(Val, -1.0f, 1.0f); }

	UFUNCTION(BlueprintCallable, Category = "Controls")
	void ShiftUp();

	UFUNCTION(BlueprintCallable, Category = "Controls")
	void ShiftDown();

	UFUNCTION(BlueprintCallable, Category = "Controls")
	void ResetBikeState();

	UFUNCTION(BlueprintPure, Category = "Telemetry")
	const FRedlineMotorcycleTelemetry& GetTelemetry() const { return Telemetry; }

	UFUNCTION(BlueprintPure, Category = "Tuning")
	const FRedlineMotorcycleTuning& GetTuning() const { return Tuning; }

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tuning")
	FRedlineMotorcycleTuning Tuning;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transmission")
	ERedlineTransmissionMode TransmissionMode = ERedlineTransmissionMode::Automatic;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRedlineGearChanged OnGearChanged;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnRedlineLowSideCrash OnLowSideCrash;

protected:
	virtual void BeginPlay() override;

	void UpdateEngineAndGearing(float DeltaTime);
	void UpdateSuspensionAndWeightShift(float DeltaTime);
	void UpdateTireFrictionAndForces(float DeltaTime);
	void UpdateAerodynamics(float DeltaTime);

private:
	float ThrottleInput = 0.0f;
	float FrontBrakeInput = 0.0f;
	float RearBrakeInput = 0.0f;
	float SteeringInput = 0.0f;

	FRedlineMotorcycleTelemetry Telemetry;

	FVector PreviousLinearVelocity = FVector::ZeroVector;
	FVector CurrentAcceleration = FVector::ZeroVector;

	UPROPERTY()
	UPrimitiveComponent* UpdatedPrimitive = nullptr;
};
