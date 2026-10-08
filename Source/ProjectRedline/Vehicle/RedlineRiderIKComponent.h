#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "RedlineRiderIKComponent.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class PROJECTREDLINE_API URedlineRiderIKComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	URedlineRiderIKComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rider IK")
	float RiderTorsoLeanDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rider IK")
	float KneeSliderExtensionDeg = 0.0f;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Rider IK")
	bool bIsKneeTouchingAsphalt = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Rider IK")
	float MaxHangOffAngleDeg = 45.0f;
};
