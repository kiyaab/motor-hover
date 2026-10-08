using UnrealBuildTool;
using System.Collections.Generic;

public class ProjectRedlineEditorTarget : TargetRules
{
	public ProjectRedlineEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V4;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("ProjectRedline");
	}
}
