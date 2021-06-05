// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "BlockData.h"

#include "BlocksDataAsset.generated.h"

USTRUCT(BlueprintType)
struct FBlockInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FLinearColor Color;
	UPROPERTY(EditAnywhere)
	bool bIsBreakable;
	UPROPERTY(EditAnywhere)
	float BreakTime;
};

UCLASS()
class MC2ELECTRICBOOGALOO_API UBlocksDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere)
	TMap<EBlockType, FBlockInfo> Blocks;
};
