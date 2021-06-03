// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MC2ElectricBoogaloo/Data/BlockData.h"

#include "Chunk.generated.h"

class UProceduralMeshComponent;
class ATerrainManager;

USTRUCT()
struct FMeshBuilder
{
	GENERATED_BODY()

	UPROPERTY()
	TArray<FVector> Vertices;
	UPROPERTY()
	TArray<int32> Triangles;
	UPROPERTY()
	TArray<FVector> Normals;
	UPROPERTY()
	TArray<FLinearColor> VertexColors;
};

UCLASS()
class MC2ELECTRICBOOGALOO_API AChunk : public AActor
{
	GENERATED_BODY()
	
public:	
	AChunk();
	UFUNCTION()
	void InitializeVariables(ATerrainManager* NewParent, const FVector2DInt& Index);

protected:
	UPROPERTY()
	ATerrainManager* Parent;
	UPROPERTY(VisibleAnywhere)
	FVector2DInt WorldIndex;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UProceduralMeshComponent* Mesh;
	
	// TODO Merge X and Y into 1 array
	UPROPERTY()
	TMap<FVectorByte, FBlock> Blocks;
	
public:
	UFUNCTION()
	FVector2DInt GetWorldPosition() const { return WorldIndex; }
	UFUNCTION()
	bool HasBlock(const FVectorByte& BlockIndex) const { return Blocks.Contains(BlockIndex); }
	UFUNCTION()
	bool GetBlockSafe(const FVectorByte& BlockIndex, FBlock& OutBlock)
	{
		if (Blocks.Contains(BlockIndex))
		{
			OutBlock = Blocks[BlockIndex];
			return true;
		}
		return false;
	}
	UFUNCTION()
	FVector GetBlockLocalPosition(const FVectorByte& BlockIndex) const;
	UFUNCTION()
	FVector GetBlockWorldPosition(const FVectorByte& BlockIndex) const;
	
	UFUNCTION()
	void AddPlane(FMeshBuilder& Builder, const FVector& Normal, const EBlockType& BlockType, const FVector& V1, const FVector& V2, const FVector& V3, const FVector& V4) const;

	UFUNCTION()
	void Rebuild(const FVector2DInt& Index);
};
