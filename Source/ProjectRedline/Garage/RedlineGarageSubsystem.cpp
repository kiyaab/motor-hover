#include "Garage/RedlineGarageSubsystem.h"

URedlineGarageSubsystem::URedlineGarageSubsystem()
{
	PlayerCredits = 10000;
}

bool URedlineGarageSubsystem::SpendCredits(int32 Amount)
{
	if (PlayerCredits >= Amount)
	{
		PlayerCredits -= Amount;
		return true;
	}
	return false;
}

bool URedlineGarageSubsystem::UpgradeEngine(int32 BikeId, int32 Cost)
{
	if (SpendCredits(Cost))
	{
		FRedlineCustomizationData& Data = BikeCustomizations.FindOrAdd(BikeId);
		if (Data.EngineUpgradeStage < 5)
		{
			Data.EngineUpgradeStage++;
			return true;
		}
	}
	return false;
}

bool URedlineGarageSubsystem::UpgradeBrakes(int32 BikeId, int32 Cost)
{
	if (SpendCredits(Cost))
	{
		FRedlineCustomizationData& Data = BikeCustomizations.FindOrAdd(BikeId);
		if (Data.BrakeUpgradeStage < 5)
		{
			Data.BrakeUpgradeStage++;
			return true;
		}
	}
	return false;
}

void URedlineGarageSubsystem::SetPaintColor(int32 BikeId, FLinearColor NewColor)
{
	FRedlineCustomizationData& Data = BikeCustomizations.FindOrAdd(BikeId);
	Data.PrimaryPaintColor = NewColor;
}
