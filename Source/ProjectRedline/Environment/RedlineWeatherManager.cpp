#include "Environment/RedlineWeatherManager.h"

ARedlineWeatherManager::ARedlineWeatherManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ARedlineWeatherManager::BeginPlay()
{
	Super::BeginPlay();
	UpdateSunPosition();
}

void ARedlineWeatherManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Advance time of day
	TimeOfDayHours += (DeltaTime / 3600.0f) * 60.0f * DayCycleSpeedMultiplier;
	if (TimeOfDayHours >= 24.0f)
	{
		TimeOfDayHours -= 24.0f;
	}

	UpdateSunPosition();
}

void ARedlineWeatherManager::UpdateSunPosition()
{
	// Sun angle based on 24 hour cycle
	const float SunPitch = (TimeOfDayHours / 24.0f) * 360.0f - 90.0f;
	SetActorRotation(FRotator(SunPitch, 45.0f, 0.0f));
}

void ARedlineWeatherManager::SetWeather(ERedlineWeatherType NewWeather)
{
	CurrentWeather = NewWeather;

	switch (CurrentWeather)
	{
	case ERedlineWeatherType::Sunny:
		RoadGripFactor = 1.0f;
		break;
	case ERedlineWeatherType::Overcast:
		RoadGripFactor = 0.95f;
		break;
	case ERedlineWeatherType::LightRain:
		RoadGripFactor = 0.72f;
		break;
	case ERedlineWeatherType::Thunderstorm:
		RoadGripFactor = 0.58f;
		break;
	case ERedlineWeatherType::Foggy:
		RoadGripFactor = 0.88f;
		break;
	}
}

void ARedlineWeatherManager::ApplyScalabilityPreset(int32 PresetLevel)
{
	// 0=Low, 1=Med, 2=High, 3=Ultra
	// Configures Lumen GI, reflections, and shadow quality
}
