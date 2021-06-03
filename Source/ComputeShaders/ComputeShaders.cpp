// Copyright 1998-2019 Epic Games, Inc. All Rights Reserved.

#include "ComputeShaders.h"
#include "Modules/ModuleManager.h"
#include "Misc/Paths.h"
#include "GlobalShader.h"

IMPLEMENT_GAME_MODULE( FComputeShadersModule, CustomShaders);


void FComputeShadersModule::StartupModule()
{
	// Maps virtual shader source directory to actual shaders directory on disk.
	FString ShaderDirectory = FPaths::Combine(FPaths::ProjectDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping("/CustomShaders", ShaderDirectory);
}

void FComputeShadersModule::ShutdownModule()
{
}

