#include "Missions/RedlineMissionSubsystem.h"
#include "Missions/RedlineMissionRegistry.h"

URedlineMissionSubsystem::URedlineMissionSubsystem()
{
}

void URedlineMissionSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	HighestUnlockedMission = 1;
	bIsMissionActive = false;
}

bool URedlineMissionSubsystem::StartMission(int32 MissionId)
{
	if (!FRedlineMissionRegistry::GetMissionById(MissionId, ActiveMission))
	{
		return false;
	}

	bIsMissionActive = true;
	MissionElapsedTime = 0.0f;
	CurrentOvertakes = 0;

	OnMissionStarted.Broadcast(ActiveMission);
	return true;
}

void URedlineMissionSubsystem::CompleteCurrentMission()
{
	if (!bIsMissionActive) return;

	bIsMissionActive = false;

	int32 Stars = 1;
	if (MissionElapsedTime <= ActiveMission.TargetTimeSeconds * 1.1f) Stars = 2;
	if (MissionElapsedTime <= ActiveMission.TargetTimeSeconds && CurrentOvertakes >= ActiveMission.TargetOvertakes) Stars = 3;

	const int32 Credits = ActiveMission.RewardCredits * Stars;

	if (ActiveMission.MissionId >= HighestUnlockedMission && HighestUnlockedMission < 100)
	{
		HighestUnlockedMission = ActiveMission.MissionId + 1;
	}

	OnMissionCompleted.Broadcast(ActiveMission.MissionId, Stars, Credits);
}

void URedlineMissionSubsystem::FailCurrentMission(FString Reason)
{
	if (!bIsMissionActive) return;
	bIsMissionActive = false;
	OnMissionFailed.Broadcast(ActiveMission.MissionId, Reason);
}

void URedlineMissionSubsystem::RecordCloseCallOvertake()
{
	if (bIsMissionActive)
	{
		CurrentOvertakes++;
	}
}
