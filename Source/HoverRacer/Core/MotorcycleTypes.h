// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "MotorcycleTypes.generated.h"

/**
 * Sequential transmission gear states.
 */
UENUM(BlueprintType)
enum class EMotorcycleGear : uint8
{
	Neutral = 0 UMETA(DisplayName = "Neutral"),
	Gear1   = 1 UMETA(DisplayName = "1st Gear"),
	Gear2   = 2 UMETA(DisplayName = "2nd Gear"),
	Gear3   = 3 UMETA(DisplayName = "3rd Gear"),
	Gear4   = 4 UMETA(DisplayName = "4th Gear"),
	Gear5   = 5 UMETA(DisplayName = "5th Gear"),
	Gear6   = 6 UMETA(DisplayName = "6th Gear")
};

/**
 * Road surface physical friction types.
 */
UENUM(BlueprintType)
enum class ERoadSurfaceType : uint8
{
	DryAsphalt   UMETA(DisplayName = "Dry Asphalt"),
	WetAsphalt   UMETA(DisplayName = "Wet Asphalt"),
	Gravel       UMETA(DisplayName = "Gravel"),
	Dirt         UMETA(DisplayName = "Dirt"),
	Grass        UMETA(DisplayName = "Grass"),
	Concrete     UMETA(DisplayName = "Concrete")
};

/**
 * Motorcycle crash states.
 */
UENUM(BlueprintType)
enum class EMotorcycleCrashType : uint8
{
	None         UMETA(DisplayName = "None"),
	LowSide      UMETA(DisplayName = "Low-Side"),
	HighSide     UMETA(DisplayName = "High-Side"),
	FrontTuck    UMETA(DisplayName = "Front Wheel Lock / Tuck"),
	Collision    UMETA(DisplayName = "Direct Collision")
};

/**
 * Physical configuration for motorcycle suspension (front fork or rear swingarm).
 */
USTRUCT(BlueprintType)
struct FMotorcycleSuspensionConfig
{
	GENERATED_BODY()

	/** Spring constant k in N/m */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension", meta = (ClampMin = "5000.0", ClampMax = "120000.0"))
	float SpringStiffness = 35000.0f;

	/** Damping coefficient c in N*s/m */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension", meta = (ClampMin = "500.0", ClampMax = "20000.0"))
	float DampingCoefficient = 3800.0f;

	/** Maximum suspension stroke travel in cm (e.g. 12 cm for front, 13 cm for rear) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension", meta = (ClampMin = "5.0", ClampMax = "30.0"))
	float MaxTravel = 12.0f;

	/** Target static sag (compression under motorcycle static weight) in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float StaticSag = 3.5f;

	/** Wheel radius in cm (standard 17-inch superbike wheel + tire ~ 31 cm radius) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel")
	float WheelRadius = 31.0f;

	/** Wheel width in cm */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wheel")
	float WheelWidth = 12.0f;
};

/**
 * Dynamic runtime state for an individual tire / suspension assembly.
 */
USTRUCT(BlueprintType)
struct FMotorcycleWheelState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wheel")
	bool bOnGround = false;

	/** Current suspension stroke compression in cm */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wheel")
	float SuspensionCompression = 0.0f;

	/** Suspension force currently applied in Newtons */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wheel")
	float SuspensionForce = 0.0f;

	/** Angular rotation velocity of wheel in rad/s */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wheel")
	float AngularVelocity = 0.0f;

	/** Longitudinal tire slip ratio (-1 = locked brake, 0 = pure roll, >0 = burnout) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wheel")
	float SlipRatio = 0.0f;

	/** Lateral tire slip angle in degrees */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wheel")
	float SlipAngleDeg = 0.0f;

	/** Contact normal of the road surface */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wheel")
	FVector ContactNormal = FVector::UpVector;

	/** Exact point where tire rubber meets road */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wheel")
	FVector ContactPoint = FVector::ZeroVector;

	/** Longitudinal driving / braking force applied in Newtons */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wheel")
	float LongitudinalForce = 0.0f;

	/** Lateral cornering force applied in Newtons */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wheel")
	float LateralForce = 0.0f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnMotorcycleCrashSignature, EMotorcycleCrashType, CrashType, float, ImpactSpeedKmh);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGearChangedSignature, EMotorcycleGear, NewGear);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnWheelieStartedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStoppieStartedSignature);
