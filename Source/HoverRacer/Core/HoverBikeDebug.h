// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HoverBikeTypes.h"

DECLARE_LOG_CATEGORY_EXTERN(LogHoverBike, Log, All);
DECLARE_LOG_CATEGORY_EXTERN(LogHoverBikePhysics, Log, All);

namespace HoverBikeDebug
{
	/**
	 * Formats physics telemetry into a clean multi-line display string for HUD or logs.
	 */
	inline FString FormatTelemetryString(
		float SpeedKmh,
		bool bGrounded,
		int32 GroundedCount,
		float HoverHeightCm,
		float LeanAngleDeg,
		float TargetLeanAngleDeg,
		float DriftAlpha,
		float GripEnergyPercent,
		float ThrustNewtons,
		float DownforceNewtons,
		const FVector& AngularVelocityRad,
		EHoverBikeState CurrentState)
	{
		const TCHAR* StateNames[] = {
			TEXT("Grounded"),
			TEXT("PartialContact"),
			TEXT("Airborne"),
			TEXT("WallRide"),
			TEXT("CeilingRide"),
			TEXT("Drifting"),
			TEXT("Crashing"),
			TEXT("Recovering"),
			TEXT("ZeroGravity")
		};

		const int32 StateIdx = static_cast<int32>(CurrentState);
		const TCHAR* StateStr = (StateIdx >= 0 && StateIdx < UE_ARRAY_COUNT(StateNames)) ? StateNames[StateIdx] : TEXT("Unknown");

		return FString::Printf(
			TEXT("================ HOVERBIKE TELEMETRY ================\n")
			TEXT(" STATE:           %s\n")
			TEXT(" SPEED:           %.1f km/h\n")
			TEXT(" GROUNDED:        %s (%d / 4 pads)\n")
			TEXT(" HOVER HEIGHT:    %.1f cm\n")
			TEXT(" LEAN (ACT/TGT):  %.1f deg / %.1f deg\n")
			TEXT(" DRIFT ALPHA:     %.2f\n")
			TEXT(" GRIP ENERGY:     %.1f %%\n")
			TEXT(" THRUST:          %.0f N\n")
			TEXT(" DOWNFORCE:       %.0f N\n")
			TEXT(" ANGULAR VEL:     X=%.2f Y=%.2f Z=%.2f rad/s\n")
			TEXT("===================================================="),
			StateStr,
			SpeedKmh,
			bGrounded ? TEXT("YES") : TEXT("NO"),
			GroundedCount,
			HoverHeightCm,
			LeanAngleDeg,
			TargetLeanAngleDeg,
			DriftAlpha,
			GripEnergyPercent,
			ThrustNewtons,
			DownforceNewtons,
			AngularVelocityRad.X, AngularVelocityRad.Y, AngularVelocityRad.Z
		);
	}
}
