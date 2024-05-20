// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class PCGTerrainTest : ModuleRules
{
	public PCGTerrainTest(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"UMG",
			"AIModule",
			"NetCore"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"RTSCore",
			"RTSCharacter",
			"RTSEntities",
			"RTSPlayer",
			"RTSTeams", 
			"RTSEquipment",
			"ModularGameplayActors",
			"CommonGame",
			"CommonUI",
			"RogueQuery"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
		
		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
