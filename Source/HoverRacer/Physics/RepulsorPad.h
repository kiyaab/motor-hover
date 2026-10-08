// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Core/HoverBikeTypes.h"

/**
 * Standard indices for the four primary repulsor pads.
 */
UENUM(BlueprintType)
enum class EHoverBikeRepulsorIndex : uint8
{
	FrontLeft  = 0 UMETA(DisplayName = "Front Left"),
	FrontRight = 1 UMETA(DisplayName = "Front Right"),
	RearLeft   = 2 UMETA(DisplayName = "Rear Left"),
	RearRight  = 3 UMETA(DisplayName = "Rear Right"),
	Count      = 4 UMETA(Hidden)
};

constexpr int32 HOVERBIKE_REPULSOR_COUNT = 4;
