// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RaceManager.generated.h"

class ACheckpointActor;
class AFinishLineActor;
class AHoverBike;

UENUM(BlueprintType)
enum class ERaceState : uint8
{
	WaitingToStart  UMETA(DisplayName = "Waiting To Start"),
	Countdown       UMETA(DisplayName = "Countdown"),
	Racing          UMETA(DisplayName = "Racing"),
	Finished        UMETA(DisplayName = "Finished")
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnCountdownStepSignature, int32, CountdownNumber);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnRaceStartedSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnLapCompletedSignature, int32, LapNumber, float, LapTime, bool, bIsBestLap);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRaceFinishedSignature, float, TotalTime, float, BestLapTime);

/**
 * Flagship Race Manager Actor.
 * Coordinates starting countdown, lap tracking, checkpoint validation, sector timing, and finish triggers.
 */
UCLASS()
class HOVERRACER_API ARaceManager : public AActor
{
	GENERATED_BODY()

public:
	ARaceManager();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;

	/** Number of laps required to complete the race */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Settings", meta = (ClampMin = "1", ClampMax = "10"))
	int32 TotalLaps = 3;

	/** Countdown duration in seconds */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Settings")
	int32 CountdownDuration = 3;

	/** Current race state */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Race|Telemetry")
	ERaceState CurrentRaceState = ERaceState::WaitingToStart;

	/** Current lap (1-indexed) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Race|Telemetry")
	int32 CurrentLap = 1;

	/** Elapsed time in current lap (seconds) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Race|Telemetry")
	float CurrentLapTime = 0.0f;

	/** Best lap time recorded this session (seconds) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Race|Telemetry")
	float BestLapTime = 0.0f;

	/** Total elapsed race time (seconds) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Race|Telemetry")
	float TotalRaceTime = 0.0f;

	/** Player's race position */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Race|Telemetry")
	int32 CurrentPosition = 1;

	/** Total competitors in the grid */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|Settings")
	int32 TotalRacers = 8;

	UFUNCTION(BlueprintCallable, Category = "Race|Flow")
	void StartCountdown();

	// Events
	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnCountdownStepSignature OnCountdownStep;

	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnRaceStartedSignature OnRaceStarted;

	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnLapCompletedSignature OnLapCompleted;

	UPROPERTY(BlueprintAssignable, Category = "Race|Events")
	FOnRaceFinishedSignature OnRaceFinished;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|References")
	TArray<TObjectPtr<ACheckpointActor>> Checkpoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Race|References")
	TObjectPtr<AFinishLineActor> FinishLine;

	UPROPERTY()
	TWeakObjectPtr<AHoverBike> PlayerHoverBike;

	UFUNCTION()
	void HandleCheckpointPassed(int32 CheckpointIndex, AActor* RacerActor, float SplitTime);

	UFUNCTION()
	void HandleFinishLineCrossed(AActor* RacerActor, float CrossingTime);

private:
	int32 NextRequiredCheckpointIndex = 0;
	float CountdownTimer = 0.0f;
	int32 LastDispatchedCountdownSecond = -1;
};
