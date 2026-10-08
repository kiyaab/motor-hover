#include "AI/RedlinePolicePursuitSubsystem.h"

URedlinePolicePursuitSubsystem::URedlinePolicePursuitSubsystem()
{
}

void URedlinePolicePursuitSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	ResetPursuit();
}

void URedlinePolicePursuitSubsystem::ReportViolation(FString ViolationType, float SeverityScore)
{
	TotalViolationScore += SeverityScore;

	ERedlinePursuitHeatLevel TargetHeat = CurrentHeat;
	if (TotalViolationScore >= 500.0f) TargetHeat = ERedlinePursuitHeatLevel::Level5;
	else if (TotalViolationScore >= 300.0f) TargetHeat = ERedlinePursuitHeatLevel::Level4;
	else if (TotalViolationScore >= 160.0f) TargetHeat = ERedlinePursuitHeatLevel::Level3;
	else if (TotalViolationScore >= 70.0f)  TargetHeat = ERedlinePursuitHeatLevel::Level2;
	else if (TotalViolationScore >= 20.0f)  TargetHeat = ERedlinePursuitHeatLevel::Level1;

	if (TargetHeat != CurrentHeat)
	{
		CurrentHeat = TargetHeat;
		CooldownTimer = BaseCooldownSeconds + (static_cast<uint8>(CurrentHeat) * 5.0f);
		OnPursuitHeatChanged.Broadcast(CurrentHeat);
	}
}

void URedlinePolicePursuitSubsystem::UpdatePursuit(float DeltaTime, bool bPlayerInLineOfSight, FVector PlayerLocation)
{
	if (CurrentHeat == ERedlinePursuitHeatLevel::None) return;

	if (bPlayerInLineOfSight)
	{
		LastKnownPlayerLocation = PlayerLocation;
		CooldownTimer = BaseCooldownSeconds + (static_cast<uint8>(CurrentHeat) * 5.0f);
	}
	else
	{
		// Player has broken line-of-sight: cooldown countdown
		CooldownTimer -= DeltaTime;
		if (CooldownTimer <= 0.0f)
		{
			ResetPursuit();
			OnPursuitEvaded.Broadcast();
		}
	}
}

void URedlinePolicePursuitSubsystem::ResetPursuit()
{
	CurrentHeat = ERedlinePursuitHeatLevel::None;
	TotalViolationScore = 0.0f;
	CooldownTimer = 0.0f;
	LastKnownPlayerLocation = FVector::ZeroVector;
	OnPursuitHeatChanged.Broadcast(CurrentHeat);
}
