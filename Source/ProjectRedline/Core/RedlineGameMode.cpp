#include "Core/RedlineGameMode.h"
#include "Vehicle/RedlineMotorcyclePawn.h"

ARedlineGameMode::ARedlineGameMode()
{
	DefaultPawnClass = ARedlineMotorcyclePawn::StaticClass();
}
