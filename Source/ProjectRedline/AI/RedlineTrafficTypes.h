#pragma once

#include "CoreMinimal.h"
#include "RedlineTrafficTypes.generated.h"

UENUM(BlueprintType)
enum class ERedlineTrafficVehicleType : uint8
{
	Sedan     UMETA(DisplayName = "City Sedan"),
	SUV       UMETA(DisplayName = "SUV"),
	SportsCar UMETA(DisplayName = "Sports Coupe"),
	SemiTruck UMETA(DisplayName = "18-Wheeler Semi-Truck")
};

UENUM(BlueprintType)
enum class ERedlinePursuitHeatLevel : uint8
{
	None     = 0 UMETA(DisplayName = "Heat 0: Clear"),
	Level1   = 1 UMETA(DisplayName = "Heat 1: Traffic Warning"),
	Level2   = 2 UMETA(DisplayName = "Heat 2: Patrol Pursuit"),
	Level3   = 3 UMETA(DisplayName = "Heat 3: Multi-Unit Interceptors"),
	Level4   = 4 UMETA(DisplayName = "Heat 4: Roadblocks & Spikes"),
	Level5   = 5 UMETA(DisplayName = "Heat 5: Citywide Lockdown")
};

USTRUCT(BlueprintType)
struct FRedlineTrafficVehicleConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	ERedlineTrafficVehicleType VehicleType = ERedlineTrafficVehicleType::Sedan;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	float DesiredSpeedKmh = 80.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	float SafeFollowDistanceM = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	float LengthCm = 440.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Traffic")
	float WidthCm = 200.0f;
};
