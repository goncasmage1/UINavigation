// Copyright (C) 2023 Gonçalo Marques - All Rights Reserved

using UnrealBuildTool;
using System.IO;

public class UINavigation : ModuleRules
{
	public UINavigation(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PrecompileForTargets = PrecompileTargetsType.Any;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"EnhancedInput",
				"ApplicationCore"
			}
			);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
                "UMG",
                "Slate",
                "SlateCore",
                "InputCore",
                "EnhancedInput",
                "RHI",
                "HeadMountedDisplay",
				"AssetRegistry"
			}
			);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
