// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class MC2ElectricBoogalooEditorTarget : TargetRules
{
	public MC2ElectricBoogalooEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange(new string[] {"MC2ElectricBoogaloo", "ComputeShaders"});
	}
}
