// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class project_k : ModuleRules
{
	public project_k(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "ProceduralMeshComponent" }); //"VTS"

		//PrivateDependencyModuleNames.AddRange(new string[] { "VTS" });


		AddEngineThirdPartyPrivateStaticDependencies(Target, "VTSLibrary");
		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
