#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "RedlineWeatherManager.generated.h"

class UDirectionalLightComponent;
class USkyLightComponent;

UENUM(BlueprintType)
enum class ERedlineWeatherType : uint8
{
	Sunny        UMETA(DisplayName = "Clear & Sunny"),
	Overcast     UMETA(DisplayName = "Overcast"),
	LightRain    UMETA(DisplayName = "Light Rain (Wet Tarmac)"),
	Thunderstorm UMETA(DisplayName = "Heavy Thunderstorm"),
	Foggy        UMETA(DisplayName = "Dense Morning Fog")
};

UCLASS()
class PROJECTREDLINE_API ARedlineWeatherManager : public AActor
{
	GENERATED_BODY()
	
public:	
	ARedlineWeatherManager();

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
	float TimeOfDayHours = 14.0f; // 2:00 PM

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Time of Day")
	float DayCycleSpeedMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weather")
	ERedlineWeatherType CurrentWeather = ERedlineWeatherType::Sunny;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Tire Physics")
	float RoadGripFactor = 1.0f;

	UFUNCTION(BlueprintCallable, Category = "Weather")
	void SetWeather(ERedlineWeatherType NewWeather);

	UFUNCTION(BlueprintCallable, Category = "Graphics")
	void ApplyScalabilityPreset(int32 PresetLevel); // 0=Low, 1=Med, 2=High, 3=Ultra

protected:
	virtual void BeginPlay() override;

private:
	void UpdateSunPosition();
};
