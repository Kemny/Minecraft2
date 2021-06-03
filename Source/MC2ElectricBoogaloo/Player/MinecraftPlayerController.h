// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "MC2ElectricBoogaloo/Data/BlockData.h"

#include "MinecraftPlayerController.generated.h"

UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	None = 0,
	Pickaxe = 1,
	PlaceBlock = 2
};

UCLASS()
class MC2ELECTRICBOOGALOO_API AMinecraftPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	EPlayerState CurrentState;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	EBlockType SelectedBlock = EBlockType::Dirt;

	virtual void Tick(float DeltaSeconds) override;
	
};
