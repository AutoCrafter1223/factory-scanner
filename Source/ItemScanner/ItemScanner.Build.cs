using UnrealBuildTool;

public class ItemScanner : ModuleRules
{
    public ItemScanner(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "DeveloperSettings",
            "DummyHeaders",
            "Engine",
            "EnhancedInput",
            "FactoryGame",
            "SML",
            "SlateCore",
            "UMG"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "InputCore",
            "Slate",
            "RenderCore",
            "RHI"
        });
    }
}
