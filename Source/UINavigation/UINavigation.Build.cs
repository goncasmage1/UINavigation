// Copyright (C) 2018 Gonçalo Marques - All Rights Reserved

using UnrealBuildTool;

public class UINavigation : ModuleRules
{
	public UINavigation(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicIncludePaths.AddRange(
			new string[] {
				"UINavigation/Public"
			}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				"UINavigation/Private"
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core"
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
                "RHI"
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
			}
			);
	}
}
