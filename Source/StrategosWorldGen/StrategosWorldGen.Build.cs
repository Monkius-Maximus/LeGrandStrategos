using UnrealBuildTool;
using System.IO;

public class StrategosWorldGen : ModuleRules
{
	public StrategosWorldGen(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		// delaunator-cpp lanca std::runtime_error (poucos pontos / colinear).
		bEnableExceptions = true;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			// O classificador de geografia emite FProvinceGeography (tipos de jogo
			// estáveis). Direção de dependência correta: experimental -> core.
			"StrategosCore"
			// "ProceduralMeshComponent" sera adicionado quando o render Opcao B
			// (mesh procedural) entrar; o render de debug atual e Opcao A (textura).
		});

		// Export de PNG do mapa de debug (FImageUtils::SaveImageByExtension).
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"ImageCore",
			"ImageWrapper"
		});

		// Header-only, MIT. Ver ThirdParty/delaunator-cpp/LICENSE.
		PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "ThirdParty", "delaunator-cpp"));
	}
}
