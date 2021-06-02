#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MC2ElectricBoogaloo/Data/Structures.h"

#include "TerrainManager.generated.h"

#define INDEX(x,y,SizeY) x * SizeY + y

class UProceduralMeshComponent;

UENUM(BlueprintType)
enum class EBlockType : uint8
{
	Air,
	Grass,
	Dirt,
	Stone,
	Sand
};

UENUM(BlueprintType)
enum class EBlockDirection : uint8
{
	Up = 0,
	Down = 1,
	Left = 2,
	Right = 3,
	Front = 4,
	Back = 5
};

USTRUCT(BlueprintType)
struct FBlock
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBlockType Type;
};

USTRUCT(BlueprintType)
struct FChunk
{
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UProceduralMeshComponent* Mesh;
	
	// TODO Merge X and Y into 1 array
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FVectorByte, FBlock> Blocks;

};

UCLASS(Blueprintable)
class MC2ELECTRICBOOGALOO_API ATerrainManager : public AActor
{
	GENERATED_BODY()

	UFUNCTION()
	static FVectorByte BlockDirectionVector(const EBlockDirection& Direction)
	{
		switch (Direction)
		{
		case EBlockDirection::Up: return FVectorByte(0,0,1);
		case EBlockDirection::Down: return FVectorByte(0,0,-1);
		case EBlockDirection::Left: return FVectorByte(0,-1,0);
		case EBlockDirection::Right: return FVectorByte(0,1,0);
		case EBlockDirection::Front: return FVectorByte(1,0,0);
		case EBlockDirection::Back: return FVectorByte(-1,0,0);
		default: return FVectorByte(0,0,0);
		}
	}
	UFUNCTION()
	static bool IsBlockTransparent(const EBlockType& Type)
	{
		switch (Type)
		{
		case EBlockType::Air: return true;
		case EBlockType::Grass: return false;
		case EBlockType::Dirt: return false;
		case EBlockType::Stone: return false;
		case EBlockType::Sand: return false;
		default: return true;
		}
	}
	
public:	
	ATerrainManager();
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInterface* TerrainMaterial;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EBlockType, FLinearColor> BlockColors;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BlockSize = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVectorByte ChunkBlockSize = {255, 255, 255};

	// Flattened array. Index is relative to player
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TMap<FVector2DInt, FChunk> Chunks;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ChunkRenderDistance = 4;

	UFUNCTION()
	static bool HasBlock(const FChunk& Chunk, const FVectorByte& Index) { return Chunk.Blocks.Contains(Index); }
	UFUNCTION()
	static bool GetBlockSafe(const FChunk& Chunk, const FVectorByte& Index, FBlock& OutBlock)
	{
		if (Chunk.Blocks.Contains(Index))
		{
			OutBlock = Chunk.Blocks[Index];
			return true;
		}
		return false;
	}
	
	UFUNCTION()
	void CreateTriangle(UProceduralMeshComponent* Mesh);

	UFUNCTION()
	void RebuildChunk(FChunk& Chunk, const FVector& WorldPosition);

	UPROPERTY(VisibleAnywhere, Category = "Debugging")
	FVector PlayerPosition;
	UPROPERTY(VisibleAnywhere, Category = "Debugging")
	FVector2DInt PlayerIndex;
};
