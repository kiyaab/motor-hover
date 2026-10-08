#pragma once

#include "CoreMinimal.h"
#include "Missions/RedlineMissionTypes.h"

class PROJECTREDLINE_API FRedlineMissionRegistry
{
public:
	static const TArray<FRedlineMissionData>& GetAllMissions();
	static bool GetMissionById(int32 MissionId, FRedlineMissionData& OutMission);
	static TArray<FRedlineMissionData> GetMissionsByChapter(int32 ChapterId);

private:
	static void InitializeRegistry();
	static TArray<FRedlineMissionData> CachedMissions;
	static bool bIsInitialized;
};
