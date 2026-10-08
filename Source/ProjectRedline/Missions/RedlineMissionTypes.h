#pragma once

#include "CoreMinimal.h"
#include "Vehicle/RedlineMotorcycleTypes.h"
#include "RedlineMissionTypes.generated.h"

UENUM(BlueprintType)
enum class ERedlineMissionType : uint8
{
	PointToPointRace    UMETA(DisplayName = "Point-to-Point Sprint"),
	CircuitRace         UMETA(DisplayName = "Circuit Grand Prix"),
	TimeTrial           UMETA(DisplayName = "Solo Time Trial"),
	CheckpointChallenge UMETA(DisplayName = "Timed Checkpoint Rush"),
	StuntChallenge      UMETA(DisplayName = "Wheelie & Stunt Challenge"),
	DeliveryCourier     UMETA(DisplayName = "High-Speed Courier"),
	RivalEncounter      UMETA(DisplayName = "1-on-1 Rival Duel"),
	PoliceEscape        UMETA(DisplayName = "Police Evade & Escape"),
	MountainHairpins    UMETA(DisplayName = "Mountain Hairpin Slalom"),
	TrafficNavigation   UMETA(DisplayName = "Highway Traffic Weaving"),
	ChampionshipFinal   UMETA(DisplayName = "Championship Apex Final")
};

USTRUCT(BlueprintType)
struct FRedlineMissionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 MissionId = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 ChapterId = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FString Title = TEXT("Welcome to Redhaven");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FString ChapterName = TEXT("Chapter 1: The Asphalt Rookie");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FString Briefing = TEXT("Get your first street bike and prove your control across the downtown boulevard.");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	ERedlineMissionType MissionType = ERedlineMissionType::TimeTrial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	ERedlineVehicleClass AllowedClass = ERedlineVehicleClass::Commuter;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	float TargetTimeSeconds = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 TargetOvertakes = 3;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	int32 RewardCredits = 1500;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Mission")
	FString LocationName = TEXT("Redhaven Downtown Boulevard");
};
