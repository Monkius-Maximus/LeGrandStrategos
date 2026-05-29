using UnrealBuildTool;

public class StrategosBattle : ModuleRules
{
	public StrategosBattle(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UMG",
			"StrategosCore",
			"StrategosData"
		});
	}
}
