#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AI/RedlineTrafficTypes.h"
#include "RedlinePolicePursuitSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPursuitHeatChanged, ERedlinePursuitHeatLevel, NewHeatLevel);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPursuitEvaded);

UCLASS()
class PROJECTREDLINE_API URedlinePolicePursuitSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	URedlinePolicePursuitSubsystem();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	UFUNCTION(BlueprintCallable, Category = "Police Pursuit")
	void ReportViolation(FString ViolationType, float SeverityScore);

	UFUNCTION(BlueprintCallable, Category = "Police Pursuit")
	void UpdatePursuit(float DeltaTime, bool bPlayerInLineOfSight, FVector PlayerLocation);

	UFUNCTION(BlueprintCallable, Category = "Police Pursuit")
	void ResetPursuit();

	UFUNCTION(BlueprintPure, Category = "Police Pursuit")
	ERedlinePursuitHeatLevel GetCurrentHeatLevel() const { return CurrentHeat; }

	UFUNCTION(BlueprintPure, Category = "Police Pursuit")
	float GetCooldownRemaining() const { return CooldownTimer; }

	UPROPERTY(BlueprintAssignable, Category = "Police Pursuit")
	FOnPursuitHeatChanged OnPursuitHeatChanged;

	UPROPERTY(BlueprintAssignable, Category = "Police Pursuit")
	FOnPursuitEvaded OnPursuitEvaded;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Police Pursuit")
	FVector LastKnownPlayerLocation = FVector::ZeroVector;

private:
	ERedlinePursuitHeatLevel CurrentHeat = ERedlinePursuitHeatLevel::None;
	float TotalViolationScore = 0.0f;
	float CooldownTimer = 0.0f;
	const float BaseCooldownSeconds = 15.0f;
};
