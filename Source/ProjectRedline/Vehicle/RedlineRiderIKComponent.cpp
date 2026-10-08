#include "Vehicle/RedlineRiderIKComponent.h"
#include "Vehicle/RedlineMotorcyclePawn.h"

URedlineRiderIKComponent::URedlineRiderIKComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void URedlineRiderIKComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ARedlineMotorcyclePawn* BikePawn = Cast<ARedlineMotorcyclePawn>(GetOwner());
	if (BikePawn && BikePawn->PhysicsComponent)
	{
		const float LeanDeg = BikePawn->PhysicsComponent->GetTelemetry().CurrentLeanDeg;
		const float LeanRatio = FMath::Clamp(LeanDeg / 58.0f, -1.0f, 1.0f);

		RiderTorsoLeanDeg = -LeanRatio * MaxHangOffAngleDeg;

		if (FMath::Abs(LeanDeg) > 28.0f)
		{
			KneeSliderExtensionDeg = FMath::Sign(LeanDeg) * 35.0f;
			bIsKneeTouchingAsphalt = (FMath::Abs(LeanDeg) > 45.0f);
		}
		else
		{
			KneeSliderExtensionDeg = 0.0f;
			bIsKneeTouchingAsphalt = false;
		}
	}
}
