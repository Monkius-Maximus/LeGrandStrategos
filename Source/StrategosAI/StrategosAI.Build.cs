using UnrealBuildTool;

public class StrategosAI : ModuleRules
{
	public StrategosAI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"StrategosCore",
			"StrategosData"
		});
	}
}
