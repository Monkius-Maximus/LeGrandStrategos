using UnrealBuildTool;
using System.Collections.Generic;

public class LeGrandStrategosTarget : TargetRules
{
	public LeGrandStrategosTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.AddRange(new string[]
		{
			"StrategosCore",
			"StrategosData",
			"StrategosBattle",
			"StrategosAI",
			"StrategosUI"
		});
	}
}
