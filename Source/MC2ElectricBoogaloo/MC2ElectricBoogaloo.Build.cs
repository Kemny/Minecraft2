// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MC2ElectricBoogaloo : ModuleRules
{
	public MC2ElectricBoogaloo(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
		PrivateDependencyModuleNames.AddRange(new string[] {"ProceduralMeshComponent" });

	}
}
