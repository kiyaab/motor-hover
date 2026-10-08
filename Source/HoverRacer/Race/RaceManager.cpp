// Copyright Epic Games, Inc. All Rights Reserved.

#include "RaceManager.h"
#include "CheckpointActor.h"
#include "FinishLineActor.h"
#include "Vehicle/HoverBike.h"
#include "Kismet/GameplayStatics.h"

ARaceManager::ARaceManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARaceManager::BeginPlay()
{
	Super::BeginPlay();

	// Locate checkpoints in level if not manually assigned
	if (Checkpoints.Num() == 0)
	{
		TArray<AActor*> FoundActors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ACheckpointActor::StaticClass(), FoundActors);
		for (AActor* Actor : FoundActors)
		{
			if (ACheckpointActor* CP = Cast<ACheckpointActor>(Actor))
			{
				Checkpoints.Add(CP);
			}
		}
		// Sort by CheckpointIndex
		Checkpoints.Sort([](const ACheckpointActor& A, const ACheckpointActor& B) {
			return A.CheckpointIndex < B.CheckpointIndex;
		});
	}

	for (ACheckpointActor* CP : Checkpoints)
	{
		if (CP)
		{
			CP->OnCheckpointPassed.AddDynamic(this, &ARaceManager::HandleCheckpointPassed);
		}
	}

	if (!FinishLine)
	{
		FinishLine = Cast<AFinishLineActor>(UGameplayStatics::GetActorOfClass(GetWorld(), AFinishLineActor::StaticClass()));
	}

	if (FinishLine)
	{
		FinishLine->OnFinishLineCrossed.AddDynamic(this, &ARaceManager::HandleFinishLineCrossed);
	}

	// Find player hoverbike
	PlayerHoverBike = Cast<AHoverBike>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));

	StartCountdown();
}

void ARaceManager::StartCountdown()
{
	CurrentRaceState = ERaceState::Countdown;
	CountdownTimer = static_cast<float>(CountdownDuration);
	LastDispatchedCountdownSecond = CountdownDuration + 1;
}

void ARaceManager::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (CurrentRaceState == ERaceState::Countdown)
	{
		CountdownTimer -= DeltaSeconds;
		const int32 DisplaySecond = FMath::CeilToInt(CountdownTimer);

		if (DisplaySecond != LastDispatchedCountdownSecond && DisplaySecond > 0)
		{
			LastDispatchedCountdownSecond = DisplaySecond;
			OnCountdownStep.Broadcast(DisplaySecond);
		}

		if (CountdownTimer <= 0.0f)
		{
			CurrentRaceState = ERaceState::Racing;
			OnRaceStarted.Broadcast();
		}
	}
	else if (CurrentRaceState == ERaceState::Racing)
	{
		CurrentLapTime += DeltaSeconds;
		TotalRaceTime += DeltaSeconds;
	}
}

void ARaceManager::HandleCheckpointPassed(int32 CheckpointIndex, AActor* RacerActor, float SplitTime)
{
	if (CurrentRaceState != ERaceState::Racing)
	{
		return;
	}

	// Verify sequential progression
	if (CheckpointIndex == NextRequiredCheckpointIndex)
	{
		NextRequiredCheckpointIndex++;
	}
}

void ARaceManager::HandleFinishLineCrossed(AActor* RacerActor, float CrossingTime)
{
	if (CurrentRaceState != ERaceState::Racing)
	{
		return;
	}

	// Must have crossed at least half of the track checkpoints to count a lap
	const int32 MinRequired = FMath::Max(1, Checkpoints.Num() / 2);
	if (NextRequiredCheckpointIndex >= MinRequired)
	{
		const bool bIsBest = (BestLapTime <= 0.0f || CurrentLapTime < BestLapTime);
		if (bIsBest)
		{
			BestLapTime = CurrentLapTime;
		}

		OnLapCompleted.Broadcast(CurrentLap, CurrentLapTime, bIsBest);

		if (CurrentLap >= TotalLaps)
		{
			CurrentRaceState = ERaceState::Finished;
			OnRaceFinished.Broadcast(TotalRaceTime, BestLapTime);
		}
		else
		{
			CurrentLap++;
			CurrentLapTime = 0.0f;
			NextRequiredCheckpointIndex = 0;
		}
	}
}
