using UnrealBuildTool;
using System.Collections.Generic;

public class ProjectRedlineTarget : TargetRules
{
	public ProjectRedlineTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("ProjectRedline");
	}
}
