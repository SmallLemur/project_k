// Copyright Epic Games, Inc. All Rights Reserved.

using System;
using System.IO;
using UnrealBuildTool;

public class VTS : ModuleRules
{
	public VTS(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

		AddEngineThirdPartyPrivateStaticDependencies(Target, "VTSLibrary");

		PublicIncludePaths.AddRange(
			new string[] {
				// ... add public include paths required here ...
				//Path.GetFullPath(Path.Combine(ModuleDirectory, "../../Source/ThirdParty/VTSLibrary/include")),
				Path.GetFullPath(Path.Combine(ModuleDirectory, "Public"))
	}
			);
				
		
		PrivateIncludePaths.AddRange(
			new string[] {
				// ... add other private include paths required here ...
				Path.GetFullPath(Path.Combine(ModuleDirectory, "Private"))
			}
			);
			
		
		PublicDependencyModuleNames.AddRange(
			new string[]
			{
				"Core",
				"VTSLibrary",
				"Projects"
				// ... add other public dependencies that you statically link with here ...
			}
			);
			
		
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"VTSLibrary",
				// ... add private dependencies that you statically link with here ...	
			}
			);
		
		
		DynamicallyLoadedModuleNames.AddRange(
			new string[]
			{
				// ... add any modules that your module loads dynamically here ...
			}
			);
		bUseRTTI = true;
		bEnableExceptions = true;
	}

}
