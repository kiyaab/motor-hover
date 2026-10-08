#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "RedlineSaveGame.generated.h"

UCLASS()
class PROJECTREDLINE_API URedlineSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	int32 PlayerCredits = 10000;

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	int32 HighestUnlockedMission = 1;

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	TMap<int32, int32> MissionStars;

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	TMap<int32, float> MissionBestTimes;

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	int32 SelectedBikeId = 0;

	UPROPERTY(VisibleAnywhere, Category = "SaveData")
	FString SaveSlotName = TEXT("RedlineSlot_0");
};
