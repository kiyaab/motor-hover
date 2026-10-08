#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Missions/RedlineMissionTypes.h"
#include "RedlineMissionSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRedlineMissionStarted, const FRedlineMissionData&, Mission);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnRedlineMissionCompleted, int32, MissionId, int32, Stars, int32, EarnedCredits);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRedlineMissionFailed, int32, MissionId, FString, Reason);

UCLASS()
class PROJECTREDLINE_API URedlineMissionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	URedlineMissionSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Missions")
	bool StartMission(int32 MissionId);

	UFUNCTION(BlueprintCallable, Category = "Missions")
	void CompleteCurrentMission();

	UFUNCTION(BlueprintCallable, Category = "Missions")
	void FailCurrentMission(FString Reason);

	UFUNCTION(BlueprintCallable, Category = "Missions")
	void RecordCloseCallOvertake();

	UFUNCTION(BlueprintPure, Category = "Missions")
	bool IsMissionActive() const { return bIsMissionActive; }

	UFUNCTION(BlueprintPure, Category = "Missions")
	const FRedlineMissionData& GetCurrentMission() const { return ActiveMission; }

	UFUNCTION(BlueprintPure, Category = "Missions")
	float GetMissionElapsedTime() const { return MissionElapsedTime; }

	UFUNCTION(BlueprintPure, Category = "Missions")
	int32 GetHighestUnlockedMissionId() const { return HighestUnlockedMission; }

	UPROPERTY(BlueprintAssignable, Category = "Missions")
	FOnRedlineMissionStarted OnMissionStarted;

	UPROPERTY(BlueprintAssignable, Category = "Missions")
	FOnRedlineMissionCompleted OnMissionCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Missions")
	FOnRedlineMissionFailed OnMissionFailed;

private:
	bool bIsMissionActive = false;
	FRedlineMissionData ActiveMission;
	float MissionElapsedTime = 0.0f;
	int32 CurrentOvertakes = 0;
	int32 HighestUnlockedMission = 1;
};
