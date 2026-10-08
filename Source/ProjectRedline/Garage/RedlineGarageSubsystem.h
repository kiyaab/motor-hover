#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Vehicle/RedlineMotorcycleTypes.h"
#include "RedlineGarageSubsystem.generated.h"

USTRUCT(BlueprintType)
struct FRedlineCustomizationData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	FLinearColor PrimaryPaintColor = FLinearColor(0.93f, 0.26f, 0.26f); // Redline Crimson

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	int32 LiveryIndex = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	int32 EngineUpgradeStage = 1; // 1 to 5

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	int32 BrakeUpgradeStage = 1;  // 1 to 5

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	int32 TireCompoundStage = 1;  // 1 to 5

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Customization")
	int32 NitroCapacityStage = 1; // 1 to 5
};

UCLASS()
class PROJECTREDLINE_API URedlineGarageSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	URedlineGarageSubsystem();

	UFUNCTION(BlueprintCallable, Category = "Garage")
	bool UpgradeEngine(int32 BikeId, int32 Cost);

	UFUNCTION(BlueprintCallable, Category = "Garage")
	bool UpgradeBrakes(int32 BikeId, int32 Cost);

	UFUNCTION(BlueprintCallable, Category = "Garage")
	void SetPaintColor(int32 BikeId, FLinearColor NewColor);

	UFUNCTION(BlueprintPure, Category = "Garage")
	int32 GetPlayerCredits() const { return PlayerCredits; }

	UFUNCTION(BlueprintCallable, Category = "Garage")
	void AddCredits(int32 Amount) { PlayerCredits += Amount; }

	UFUNCTION(BlueprintCallable, Category = "Garage")
	bool SpendCredits(int32 Amount);

private:
	int32 PlayerCredits = 5000;
	TMap<int32, FRedlineCustomizationData> BikeCustomizations;
};
