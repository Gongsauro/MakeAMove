// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MakeAMove : ModuleRules
{
	public MakeAMove(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] {
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"AIModule",
			"StateTreeModule",
			"GameplayStateTreeModule",
			"UMG",
			"Slate"
		});

		PrivateDependencyModuleNames.AddRange(new string[] { });

		PublicIncludePaths.AddRange(new string[] {
			"MakeAMove",
			"MakeAMove/Variant_Platforming",
			"MakeAMove/Variant_Platforming/Animation",
			"MakeAMove/Variant_Combat",
			"MakeAMove/Variant_Combat/AI",
			"MakeAMove/Variant_Combat/Animation",
			"MakeAMove/Variant_Combat/Gameplay",
			"MakeAMove/Variant_Combat/Interfaces",
			"MakeAMove/Variant_Combat/UI",
			"MakeAMove/Variant_SideScrolling",
			"MakeAMove/Variant_SideScrolling/AI",
			"MakeAMove/Variant_SideScrolling/Gameplay",
			"MakeAMove/Variant_SideScrolling/Interfaces",
			"MakeAMove/Variant_SideScrolling/UI"
		});

		// Uncomment if you are using Slate UI
		// PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true
	}
}
