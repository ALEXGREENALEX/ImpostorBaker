using UnrealBuildTool;

public class ImpostorBakerEditor : ModuleRules
{
	public ImpostorBakerEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		PublicIncludePaths.Add(ModuleDirectory);

		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"CoreUObject",
				"Engine",
				"Slate",
				"SlateCore",
				"UnrealEd",
				"MeshDescription",
				"DeveloperSettings",
				"CommonMenuExtensions",
				"AdvancedPreviewScene",
				"ProceduralMeshComponent",
				"RHI",
			}
		);
	}
}