// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once 

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "MC2ElectricBoogalooHUD.generated.h"

UCLASS()
class AMC2ElectricBoogalooHUD : public AHUD
{
	GENERATED_BODY()

public:
	AMC2ElectricBoogalooHUD();

	/** Primary draw call for the HUD */
	virtual void DrawHUD() override;

private:
	/** Crosshair asset pointer */
	class UTexture2D* CrosshairTex;

};

