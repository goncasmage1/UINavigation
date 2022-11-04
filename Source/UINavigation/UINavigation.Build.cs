// Copyright (C) 2019 Gonçalo Marques - All Rights Reserved

using UnrealBuildTool;
using System.IO;

public class UINavigation : ModuleRules
{
	public UINavigation(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"EnhancedInput"
			}
			);
			
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"ApplicationCore",
				"Engine",
                "UMG",
                "Slate",
                "SlateCore",
                "InputCore",
                "EnhancedInput",
                "RHI",
                "HeadMountedDisplay"
			}
			);
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
