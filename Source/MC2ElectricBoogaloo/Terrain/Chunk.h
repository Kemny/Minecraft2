// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MC2ElectricBoogaloo/Data/BlockData.h"

#include "Chunk.generated.h"

class UProceduralMeshComponent;
class ATerrainManager;

USTRUCT()
struct FMeshInfo
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

	void Clear()
	{
		Vertices.Empty();
		Triangles.Empty();
		Normals.Empty();
		VertexColors.Empty();
	}
};

DECLARE_DYNAMIC_DELEGATE_TwoParams(FChunkUpdateDelegateInternal, const TArray<FMeshInfo>&, MeshInfos, const TArray<FVector2DInt>&, MissingChunkDirections);
DECLARE_DYNAMIC_DELEGATE_TwoParams(FChunkEdgeUpdateDelegate, const FVector2DInt&, Index, const TArray<FVector2DInt>&, Directions);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FChunkUpdateDelegale, const FVector2DInt&, Index);

UCLASS()
class MC2ELECTRICBOOGALOO_API AChunk : public AActor
{
	GENERATED_BODY()
	
public:	
	AChunk();
	
	UFUNCTION()
	void InitializeVariables(ATerrainManager* NewParent);

	UPROPERTY()
	FChunkUpdateDelegale OnUpdated;
	UPROPERTY()
	FChunkEdgeUpdateDelegate OnEdgeUpdated;
	
protected:
	UPROPERTY()
	ATerrainManager* Parent;
	
	UPROPERTY(VisibleAnywhere)
	FVector2DInt WorldIndex;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UProceduralMeshComponent* Mesh;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInterface* Material;
	
	// TODO Merge X and Y into 1 array
	UPROPERTY()
	TMap<FVectorByte, FBlock> Blocks;

	// One Per Direction
	UPROPERTY()
	TArray<FMeshInfo> MeshInfos;

	UPROPERTY()
	bool bIsDirty = false;

	UPROPERTY()
	TArray<FVector2DInt> MissingChunkDirections;
	
	FThreadSafeBool bCancelThread;
	FThreadSafeBool bIsThreadRunning;

	UFUNCTION()
	void OnBuildThreadFinished(const TArray<FMeshInfo>& NewMeshInfos, const TArray<FVector2DInt>& MissingDirections);
	
public:
	UFUNCTION()
	void GetMissingChunkDirections(TArray<FVector2DInt>& Indexes) const { Indexes = MissingChunkDirections; }
	
	UFUNCTION()
	bool IsReady() const { return bIsThreadRunning; }
	
	UFUNCTION()
	FVector2DInt GetWorldIndex() const { return WorldIndex; }
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
	bool GetBlockTypeSafe(const FVectorByte& BlockIndex, EBlockType& OutBlockType)
	{
		if (Blocks.Contains(BlockIndex))
		{
			OutBlockType = Blocks[BlockIndex].Type;
			return true;
		}
		return false;
	}
	
	UFUNCTION()
	bool ContainsBlock(const FVectorByte& BlockIndex) const
	{
		return Blocks.Contains(BlockIndex);
	}
	
	UFUNCTION()
	FVector GetBlockLocalPosition(const FVectorByte& BlockIndex) const;
	UFUNCTION()
	FVector GetBlockWorldPosition(const FVectorByte& BlockIndex) const;

	UFUNCTION()
	void BuildBlocks(const FVector2DInt& Index);
	UFUNCTION()
	void RebuildGeometry();

	UFUNCTION()
	void RemoveBlock(const FVectorByte& BlockIndex);
	UFUNCTION()
	void AddBlock(const FVectorByte& BlockIndex, const EBlockType Type);

	UFUNCTION()
	bool IsBlockEdge(const FVectorByte& BlockIndex, TArray<FVector2DInt>& EdgeDirections) const;
};
