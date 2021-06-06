// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"

#include "MC2ElectricBoogaloo/Data/BlockData.h"

#include "ChunkSaveGame.generated.h"

UCLASS()
class MC2ELECTRICBOOGALOO_API UChunkSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UFUNCTION()
	static void SaveChunk(const FString& SaveName, const FVector2DInt& ChunkIndex, const TMap<FVectorByte, FBlock>& NewBlocks);
	
	UFUNCTION()
	static bool TryToLoadChunk(const FString& SaveName, const FVector2DInt& ChunkIndex, TMap<FVectorByte, FBlock>& NewBlocks);
	
	UPROPERTY(SaveGame)
	TMap<FVectorByte, FBlock> Blocks;
};
