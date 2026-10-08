// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Core/MotorcycleTypes.h"
#include "MotorcyclePawn.generated.h"

class UStaticMeshComponent;
class USpringArmComponent;
class UCameraComponent;
class UMotorcyclePhysicsComponent;
class UMotorcycleEngineComponent;
class UMotorcycleTireComponent;

/**
 * Flagship physically simulated 2-wheel racing motorcycle pawn.
 */
UCLASS()
class HOVERRACER_API AMotorcyclePawn : public APawn
{
	GENERATED_BODY()

public:
	AMotorcyclePawn();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void Tick(float DeltaSeconds) override;

	// Component Getters
	UFUNCTION(BlueprintPure, Category = "Motorcycle|Components")
	UStaticMeshComponent* GetChassisMesh() const { return ChassisMesh; }

	UFUNCTION(BlueprintPure, Category = "Motorcycle|Components")
	UMotorcyclePhysicsComponent* GetPhysicsComponent() const { return PhysicsComponent; }

	UFUNCTION(BlueprintPure, Category = "Motorcycle|Components")
	UMotorcycleEngineComponent* GetEngineComponent() const { return EngineComponent; }

	UFUNCTION(BlueprintPure, Category = "Motorcycle|Components")
	UMotorcycleTireComponent* GetFrontWheel() const { return FrontWheel; }

	UFUNCTION(BlueprintPure, Category = "Motorcycle|Components")
	UMotorcycleTireComponent* GetRearWheel() const { return RearWheel; }

	// Control Interface
	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Control")
	void SetThrottle(float Value);

	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Control")
	void SetFrontBrake(float Value);

	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Control")
	void SetRearBrake(float Value);

	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Control")
	void SetSteer(float Value);

	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Control")
	void ShiftUp();

	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Control")
	void ShiftDown();

	UFUNCTION(BlueprintCallable, Category = "Motorcycle|Control")
	void ResetBike();

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motorcycle|Components")
	TObjectPtr<UStaticMeshComponent> ChassisMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motorcycle|Components")
	TObjectPtr<UMotorcycleTireComponent> FrontWheel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motorcycle|Components")
	TObjectPtr<UMotorcycleTireComponent> RearWheel;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motorcycle|Components")
	TObjectPtr<UMotorcycleEngineComponent> EngineComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motorcycle|Components")
	TObjectPtr<UMotorcyclePhysicsComponent> PhysicsComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motorcycle|Components")
	TObjectPtr<USpringArmComponent> CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Motorcycle|Components")
	TObjectPtr<UCameraComponent> Camera;

private:
	void UpdateCameraDynamics(float DeltaSeconds);
};
