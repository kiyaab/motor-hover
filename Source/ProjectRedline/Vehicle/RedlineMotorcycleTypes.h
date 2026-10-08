#pragma once

#include "CoreMinimal.h"
#include "RedlineMotorcycleTypes.generated.h"

UENUM(BlueprintType)
enum class ERedlineTransmissionMode : uint8
{
	Automatic UMETA(DisplayName = "Automatic"),
	Manual    UMETA(DisplayName = "Manual")
};

UENUM(BlueprintType)
enum class ERedlineVehicleClass : uint8
{
	Commuter   UMETA(DisplayName = "Commuter (125cc-250cc)"),
	Naked      UMETA(DisplayName = "Naked Streetfighter (650cc-900cc)"),
	Sport      UMETA(DisplayName = "Supersport (600cc)"),
	Superbike  UMETA(DisplayName = "Superbike (1000cc)"),
	Touring    UMETA(DisplayName = "Grand Touring (1200cc)"),
	Adventure  UMETA(DisplayName = "Adventure Dual-Sport"),
	Dirt       UMETA(DisplayName = "Off-Road Dirt Bike"),
	Cruiser    UMETA(DisplayName = "Custom V-Twin Cruiser"),
	Electric   UMETA(DisplayName = "High-Voltage Electric Superbike")
};

UENUM(BlueprintType)
enum class ERedlineRoadSurface : uint8
{
	DryAsphalt UMETA(DisplayName = "Dry Asphalt"),
	WetAsphalt UMETA(DisplayName = "Wet Asphalt"),
	Gravel     UMETA(DisplayName = "Gravel / Dirt"),
	Mud        UMETA(DisplayName = "Mud"),
	Grass      UMETA(DisplayName = "Grass")
};

USTRUCT(BlueprintType)
struct FRedlineMotorcycleTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass & Geometry")
	float BikeMassKg = 195.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass & Geometry")
	float RiderMassKg = 75.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass & Geometry")
	float WheelbaseCm = 144.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass & Geometry")
	float CenterOfMassHeightCm = 56.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass & Geometry")
	float WheelRadiusCm = 31.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mass & Geometry")
	float MaxLeanAngleDeg = 58.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powertrain")
	float IdleRPM = 1200.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powertrain")
	float PeakTorqueRPM = 11000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powertrain")
	float RedlineRPM = 14500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powertrain")
	float MaxRPM = 15500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powertrain")
	float PeakTorqueNm = 118.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powertrain")
	float PrimaryRatio = 1.681f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powertrain")
	float FinalRatio = 2.625f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Powertrain")
	TArray<float> ForwardGearRatios = { 2.562f, 2.052f, 1.714f, 1.500f, 1.360f, 1.269f };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brakes")
	float FrontBrakeMaxTorqueNm = 1250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Brakes")
	float RearBrakeMaxTorqueNm = 450.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aerodynamics")
	float DragAreaCdA = 0.42f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Aerodynamics")
	float TuckedInDragAreaCdA = 0.31f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float FrontSuspensionRestCm = 12.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float FrontSpringRateNPerCm = 190.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float RearSuspensionRestCm = 13.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Suspension")
	float RearSpringRateNPerCm = 220.0f;
};

USTRUCT(BlueprintType)
struct FRedlineMotorcycleTelemetry
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	float SpeedKmh = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	float EngineRPM = 1200.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	int32 CurrentGear = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	float CurrentLeanDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	float FrontForkCompressionCm = 3.5f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	float RearShockCompressionCm = 3.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	bool bIsWheelie = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	bool bIsStoppie = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	bool bIsLowSideSlide = false;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	float FrontTireGripFactor = 1.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Telemetry")
	float RearTireGripFactor = 1.0f;
};
