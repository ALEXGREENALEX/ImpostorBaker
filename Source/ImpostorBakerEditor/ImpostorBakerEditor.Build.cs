using UnrealBuildTool;

public class ImpostorBakerEditor : ModuleRules
{
    public ImpostorBakerEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
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
            }
        );
    }
}