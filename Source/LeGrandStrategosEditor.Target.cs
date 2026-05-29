using UnrealBuildTool;
using System.Collections.Generic;

public class LeGrandStrategosEditorTarget : TargetRules
{
	public LeGrandStrategosEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[]
		{
			"StrategosCore",
			"StrategosData",
			"StrategosBattle",
			"StrategosAI",
			"StrategosUI",
			"StrategosWorldGen"
		});
	}
}
