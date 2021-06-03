// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ComputeShaders : ModuleRules
{
	public ComputeShaders(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { });
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"Renderer",
			"RenderCore",
			"RHI",
			"Projects"
		});
	}
}
