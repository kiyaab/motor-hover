#include "Missions/RedlineMissionRegistry.h"

TArray<FRedlineMissionData> FRedlineMissionRegistry::CachedMissions;
bool FRedlineMissionRegistry::bIsInitialized = false;

const TArray<FRedlineMissionData>& FRedlineMissionRegistry::GetAllMissions()
{
	if (!bIsInitialized)
	{
		InitializeRegistry();
	}
	return CachedMissions;
}

bool FRedlineMissionRegistry::GetMissionById(int32 MissionId, FRedlineMissionData& OutMission)
{
	const TArray<FRedlineMissionData>& All = GetAllMissions();
	for (const FRedlineMissionData& M : All)
	{
		if (M.MissionId == MissionId)
		{
			OutMission = M;
			return true;
		}
	}
	return false;
}

TArray<FRedlineMissionData> FRedlineMissionRegistry::GetMissionsByChapter(int32 ChapterId)
{
	TArray<FRedlineMissionData> Result;
	const TArray<FRedlineMissionData>& All = GetAllMissions();
	for (const FRedlineMissionData& M : All)
	{
		if (M.ChapterId == ChapterId)
		{
			Result.Add(M);
		}
	}
	return Result;
}

void FRedlineMissionRegistry::InitializeRegistry()
{
	CachedMissions.Empty();
	CachedMissions.Reserve(100);

	struct FChapterMeta
	{
		int32 Id;
		FString Name;
		FString Location;
		ERedlineVehicleClass ClassType;
	};

	const FChapterMeta Chapters[10] = {
		{ 1,  TEXT("Chapter 1: The Asphalt Rookie"),           TEXT("Downtown Financial District"),    ERedlineVehicleClass::Commuter },
		{ 2,  TEXT("Chapter 2: Redhaven Underground"),         TEXT("Commercial Streets & Tunnels"),   ERedlineVehicleClass::Naked },
		{ 3,  TEXT("Chapter 3: Harbor Speed Trials"),          TEXT("Shipping Docks & Container Yards"),ERedlineVehicleClass::Naked },
		{ 4,  TEXT("Chapter 4: Neon Syndicate"),               TEXT("Tokyo Neon Expressway"),          ERedlineVehicleClass::Sport },
		{ 5,  TEXT("Chapter 5: Mountain Hairpins & Drift"),    TEXT("Highland Mountain Pass"),         ERedlineVehicleClass::Sport },
		{ 6,  TEXT("Chapter 6: High Stakes Highway Outlaws"),  TEXT("Coastal Highway & Overpasses"),   ERedlineVehicleClass::Superbike },
		{ 7,  TEXT("Chapter 7: Desert Endurance"),             TEXT("Dusty Valley & Rural Villages"),  ERedlineVehicleClass::Adventure },
		{ 8,  TEXT("Chapter 8: The Syndicate Betrayal"),       TEXT("Industrial District & Warehouses"),ERedlineVehicleClass::Superbike },
		{ 9,  TEXT("Chapter 9: Police Lockdown Gauntlet"),     TEXT("Citywide Tactical Barricades"),   ERedlineVehicleClass::Superbike },
		{ 10, TEXT("Chapter 10: Grand Apex Championship"),      TEXT("Redhaven International Circuit"),  ERedlineVehicleClass::Superbike }
	};

	const TCHAR* MissionTitles[100] = {
		// Chapter 1 (1-10)
		TEXT("First Ignition"), TEXT("Redline Delivery"), TEXT("Downtown Sprint"), TEXT("Traffic Weave 101"), TEXT("Alleyway Shortcut"),
		TEXT("Late Night Rush"), TEXT("Speed Trap Test"), TEXT("Pavement Fever"), TEXT("Rival on the Strip"), TEXT("Rookie License Exam"),
		// Chapter 2 (11-20)
		TEXT("Underground Ignition"), TEXT("Subway Tunnel Dash"), TEXT("Midnight Cruise"), TEXT("Concrete Canyon"), TEXT("Commercial Slalom"),
		TEXT("Rooftop Ramp Jump"), TEXT("Neon Alley Sprint"), TEXT("Courier Run: The Package"), TEXT("The Informant's Chase"), TEXT("Underground Crown"),
		// Chapter 3 (21-30)
		TEXT("Salt Air Quarter Mile"), TEXT("Container Maze"), TEXT("Dockside Drag"), TEXT("Crane Shadow Drift"), TEXT("Pier to Pier Sprint"),
		TEXT("Cargo Run Heist"), TEXT("Harbor Patrol Evade"), TEXT("Wet Tarmac Reflexes"), TEXT("The Dockmaster's Challenge"), TEXT("King of the Port"),
		// Chapter 4 (31-40)
		TEXT("Neon Skyline Entry"), TEXT("Midnight Overpass"), TEXT("Syndicate Line"), TEXT("High Voltage Squeeze"), TEXT("Cyber Avenue Rush"),
		TEXT("Expressway Ghost"), TEXT("Adrenaline Rush"), TEXT("The Double Apex"), TEXT("Shadow Duel: Katsu"), TEXT("Neon Master"),
		// Chapter 5 (41-50)
		TEXT("Incline Ascent"), TEXT("Hairpin 7"), TEXT("Clifftop Edge"), TEXT("Mountain Fog Descent"), TEXT("Knee Scraper"),
		TEXT("Switchback Slalom"), TEXT("Alpine Speed Trap"), TEXT("Gravel Edge Hazard"), TEXT("The Canyon Phantom"), TEXT("Mountain Summit Crown"),
		// Chapter 6 (51-60)
		TEXT("Coastal 300"), TEXT("Highway Blindspot"), TEXT("Gridlock Escape"), TEXT("Semi-Truck Squeeze"), TEXT("High Stakes Wager"),
		TEXT("Radar Buster"), TEXT("Sunset Boulevard"), TEXT("Nightmare Traffic"), TEXT("Rival Duel: Torretto"), TEXT("Highway Outlaw Legend"),
		// Chapter 7 (61-70)
		TEXT("Dust and Asphalt"), TEXT("Rural Gas Station Run"), TEXT("Baja Ridge Sprint"), TEXT("Farmland Crossroads"), TEXT("Desert Oasis Rush"),
		TEXT("Abandoned Airfield"), TEXT("Canyon Echoes"), TEXT("Sunstroke Rally"), TEXT("Dust Storm Survival"), TEXT("Desert King"),
		// Chapter 8 (71-80)
		TEXT("The Setup"), TEXT("Warehouse Infiltration"), TEXT("Industrial Rail Yard"), TEXT("Double Crossed"), TEXT("Factory Firestorm"),
		TEXT("The Stolen Prototype"), TEXT("Escape the Docks"), TEXT("Alleyway Ambush"), TEXT("Syndicate Boss Duel"), TEXT("Retribution"),
		// Chapter 9 (81-90)
		TEXT("Code Red Alert"), TEXT("Spike Strip Slalom"), TEXT("Helicopter Spotlight"), TEXT("Bridge Barricade Breach"), TEXT("Lockdown Breakout"),
		TEXT("Five Star Pursuit"), TEXT("Tunnel Blockade Blast"), TEXT("Urban Evasion"), TEXT("Tactical Interceptor Duel"), TEXT("Free of the Net"),
		// Chapter 10 (91-100)
		TEXT("Apex Qualifiers"), TEXT("Circuit Warmup"), TEXT("Rain GP Trial"), TEXT("Speed Kings Gathering"), TEXT("The 200 MPH Barrier"),
		TEXT("Penultimate Lap"), TEXT("The Final Grid"), TEXT("Championship Thunder"), TEXT("Clash of Legends"), TEXT("The Apex Legend: 100th Milestone")
	};

	const ERedlineMissionType MissionTypesPattern[10] = {
		ERedlineMissionType::TimeTrial,
		ERedlineMissionType::DeliveryCourier,
		ERedlineMissionType::PointToPointRace,
		ERedlineMissionType::TrafficNavigation,
		ERedlineMissionType::CheckpointChallenge,
		ERedlineMissionType::CircuitRace,
		ERedlineMissionType::StuntChallenge,
		ERedlineMissionType::PoliceEscape,
		ERedlineMissionType::RivalEncounter,
		ERedlineMissionType::ChampionshipFinal
	};

	for (int32 i = 0; i < 100; i++)
	{
		const int32 MissionId = i + 1;
		const int32 ChapterIdx = i / 10;
		const int32 MissionInChapter = i % 10;
		const FChapterMeta& Meta = Chapters[ChapterIdx];

		FRedlineMissionData Data;
		Data.MissionId = MissionId;
		Data.ChapterId = Meta.Id;
		Data.Title = FString(MissionTitles[i]);
		Data.ChapterName = Meta.Name;
		Data.LocationName = Meta.Location;
		Data.AllowedClass = Meta.ClassType;
		Data.MissionType = MissionTypesPattern[MissionInChapter];
		Data.TargetTimeSeconds = 30.0f + (MissionId * 1.5f);
		Data.TargetOvertakes = 2 + (MissionId / 8);
		Data.RewardCredits = 1000 + (MissionId * 250);
		Data.Briefing = FString::Printf(TEXT("Mission %d: %s. Prove your mastery on the %s roads."), MissionId, MissionTitles[i], *Meta.Location);

		CachedMissions.Add(Data);
	}

	bIsInitialized = true;
}
