#pragma once

#include "CoreMinimal.h"

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class COMPUTESHADERS_API FComputeShadersModule : public IModuleInterface
{
	public:
	static inline FComputeShadersModule& Get()
	{
		return FModuleManager::LoadModuleChecked<FComputeShadersModule>("ComputeShaders");
	}

	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("ComputeShaders");
	}

	public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
