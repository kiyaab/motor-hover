// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "HoverBikeTypes.generated.h"

/**
 * State machine representing the vehicle's dynamic physical mode.
 */
UENUM(BlueprintType)
enum class EHoverBikeState : uint8
{
	Grounded        UMETA(DisplayName = "Grounded"),
	PartialContact  UMETA(DisplayName = "Partial Contact"),
	Airborne        UMETA(DisplayName = "Airborne"),
	WallRide        UMETA(DisplayName = "Wall Ride"),
	CeilingRide     UMETA(DisplayName = "Ceiling Ride"),
	Drifting        UMETA(DisplayName = "Drifting"),
	Crashing        UMETA(DisplayName = "Crashing"),
	Recovering      UMETA(DisplayName = "Recovering"),
	ZeroGravity     UMETA(DisplayName = "Zero Gravity")
};

/**
 * Static configuration parameters for an individual repulsor pad.
 */
USTRUCT(BlueprintType)
struct FRepulsorPadConfig
{
	GENERATED_BODY()

	/** Local coordinate offset from vehicle origin (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	FVector LocalOffset = FVector::ZeroVector;

	/** Target equilibrium spring length / hover height (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension", meta = (ClampMin = "10.0", ClampMax = "500.0"))
	float RestLength = 150.0f;

	/** Maximum downward trace sweep distance (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension", meta = (ClampMin = "20.0", ClampMax = "1000.0"))
	float MaxTraceLength = 250.0f;

	/** Radius of the sphere sweep (cm) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension", meta = (ClampMin = "1.0", ClampMax = "100.0"))
	float TraceRadius = 15.0f;

	/** Spring constant k (N/m scaled for UE units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension", meta = (ClampMin = "1000.0", ClampMax = "2000000.0"))
	float SpringStiffness = 250000.0f;

	/** Damper coefficient c (N*s/m scaled for UE units) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension", meta = (ClampMin = "100.0", ClampMax = "200000.0"))
	float DampingCoefficient = 18000.0f;

	/** Maximum force clamp for this repulsor pad (N) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension", meta = (ClampMin = "1000.0", ClampMax = "1000000.0"))
	float MaxForce = 300000.0f;

	/** Time to remember contact after losing trace (seconds) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ContactGraceTime = 0.15f;

	/** Weight of this pad in the blended surface normal calculation */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension", meta = (ClampMin = "0.0", ClampMax = "2.0"))
	float SurfaceNormalWeight = 1.0f;

	/** Enable debug visualization for this pad */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	bool bEnableDebugDraw = true;
};

/**
 * Dynamic runtime state for an individual repulsor pad.
 */
USTRUCT(BlueprintType)
struct FRepulsorPadState
{
	GENERATED_BODY()

	/** True if the pad has detected a valid driving surface within range */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Suspension")
	bool bGrounded = false;

	/** Current distance from repulsor mount to surface (cm) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Suspension")
	float CurrentLength = 150.0f;

	/** Spring compression (RestLength - CurrentLength) (cm) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Suspension")
	float Compression = 0.0f;

	/** Rate of change of compression (cm/s) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Suspension")
	float CompressionVelocity = 0.0f;

	/** World position of the contact point */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Suspension")
	FVector HitLocation = FVector::ZeroVector;

	/** World normal of the contacted surface */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Suspension")
	FVector SurfaceNormal = FVector::UpVector;

	/** Velocity vector of the rigid body at the repulsor mount location (cm/s) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Suspension")
	FVector ContactVelocity = FVector::ZeroVector;

	/** Magnitude of suspension force applied this step (N) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Suspension")
	float AppliedForce = 0.0f;

	/** Time elapsed since last grounded contact (seconds) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Suspension")
	float TimeSinceGrounded = 0.0f;
};

/**
 * Serializable physics state snapshot for networking, AI telemetry, and replay.
 */
USTRUCT(BlueprintType)
struct FHoverBikePhysicsState
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	FVector Position = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	FQuat Rotation = FQuat::Identity;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	FVector LinearVelocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	FVector AngularVelocity = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	FVector SurfaceNormal = FVector::UpVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	float Throttle = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	float Steering = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	float Drift = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	uint8 GroundedPads = 0;
};

// ==========================================
// Custom Gravity Interface
// ==========================================

UINTERFACE(MinimalAPI, BlueprintType)
class UGravityProviderInterface : public UInterface
{
	GENERATED_BODY()
};

class HOVERRACER_API IGravityProviderInterface
{
	GENERATED_BODY()

public:
	/** Returns the gravity acceleration vector at the given world position (cm/s^2) */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Gravity")
	FVector GetGravityAtLocation(const FVector& WorldLocation) const;
};

// ==========================================
// Gameplay & VFX Event Delegates
// ==========================================

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRepulsorContactSignature, int32, PadIndex, const FVector&, HitLocation);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRepulsorLostSignature, int32, PadIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDriftStartedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnDriftEndedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBoostStartedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnBoostEndedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAirborneSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLandingSignature, float, ImpactVelocityNormal);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCrashSignature, float, Severity, const FVector&, ImpactPoint);
