// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;

public class InputSetup : ModuleRules
{
	public InputSetup(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;
		PrivatePCHHeaderFile = "Private/InputSetupPrivatePCH.h";

		PublicDependencyModuleNames.AddRange(new string[] { "Core" });
		PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"CoreUObject",
				"Engine",
				"EnhancedInput"
			}
		);
	}
}
