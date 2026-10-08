// Copyright Epic Games, Inc. All Rights Reserved.

#include "HoverBikeTuningData.h"

UHoverBikeTuningData::UHoverBikeTuningData()
{
	RepulsorConfigs.SetNum(4);

	// Pad 0: Front-Left
	RepulsorConfigs[0].LocalOffset = FVector(140.0f, -45.0f, -20.0f);
	RepulsorConfigs[0].RestLength = 150.0f;
	RepulsorConfigs[0].MaxTraceLength = 250.0f;
	RepulsorConfigs[0].TraceRadius = 15.0f;
	RepulsorConfigs[0].SpringStiffness = 250000.0f;
	RepulsorConfigs[0].DampingCoefficient = 18000.0f;
	RepulsorConfigs[0].MaxForce = 300000.0f;
	RepulsorConfigs[0].ContactGraceTime = 0.15f;
	RepulsorConfigs[0].SurfaceNormalWeight = 1.0f;
	RepulsorConfigs[0].bEnableDebugDraw = true;

	// Pad 1: Front-Right
	RepulsorConfigs[1].LocalOffset = FVector(140.0f, 45.0f, -20.0f);
	RepulsorConfigs[1].RestLength = 150.0f;
	RepulsorConfigs[1].MaxTraceLength = 250.0f;
	RepulsorConfigs[1].TraceRadius = 15.0f;
	RepulsorConfigs[1].SpringStiffness = 250000.0f;
	RepulsorConfigs[1].DampingCoefficient = 18000.0f;
	RepulsorConfigs[1].MaxForce = 300000.0f;
	RepulsorConfigs[1].ContactGraceTime = 0.15f;
	RepulsorConfigs[1].SurfaceNormalWeight = 1.0f;
	RepulsorConfigs[1].bEnableDebugDraw = true;

	// Pad 2: Rear-Left
	RepulsorConfigs[2].LocalOffset = FVector(-140.0f, -50.0f, -20.0f);
	RepulsorConfigs[2].RestLength = 150.0f;
	RepulsorConfigs[2].MaxTraceLength = 250.0f;
	RepulsorConfigs[2].TraceRadius = 15.0f;
	RepulsorConfigs[2].SpringStiffness = 250000.0f;
	RepulsorConfigs[2].DampingCoefficient = 18000.0f;
	RepulsorConfigs[2].MaxForce = 300000.0f;
	RepulsorConfigs[2].ContactGraceTime = 0.15f;
	RepulsorConfigs[2].SurfaceNormalWeight = 1.0f;
	RepulsorConfigs[2].bEnableDebugDraw = true;

	// Pad 3: Rear-Right
	RepulsorConfigs[3].LocalOffset = FVector(-140.0f, 50.0f, -20.0f);
	RepulsorConfigs[3].RestLength = 150.0f;
	RepulsorConfigs[3].MaxTraceLength = 250.0f;
	RepulsorConfigs[3].TraceRadius = 15.0f;
	RepulsorConfigs[3].SpringStiffness = 250000.0f;
	RepulsorConfigs[3].DampingCoefficient = 18000.0f;
	RepulsorConfigs[3].MaxForce = 300000.0f;
	RepulsorConfigs[3].ContactGraceTime = 0.15f;
	RepulsorConfigs[3].SurfaceNormalWeight = 1.0f;
	RepulsorConfigs[3].bEnableDebugDraw = true;
}
