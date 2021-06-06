#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MC2ElectricBoogaloo/Data/BlocksDataAsset.h"

#include "TerrainManager.generated.h"

#define INDEX(x,y,SizeY) x * SizeY + y

class AMinecraftPlayerController;
class AChunk;
class UBlocksDataAsset;

DECLARE_DYNAMIC_DELEGATE(FTerrainDelegate);

USTRUCT()
struct FChunkUpdateWaiters
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	TArray<AChunk*> Chunks;
};

UCLASS(Blueprintable)
class MC2ELECTRICBOOGALOO_API ATerrainManager : public AActor
{
	GENERATED_BODY()

public:
	
	ATerrainManager();
	UFUNCTION(BlueprintCallable)
	void CreateTerrain(AMinecraftPlayerController* Player, const FString& SaveName, FTerrainDelegate OnGenerated);

	UPROPERTY()
	FTerrainDelegate OnTerrainGenerated;
	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	float GetNoisePersistence() const { return NoisePersistence; }
	UFUNCTION()
	float GetNoiseDamper() const { return NoiseDamper; }
	UFUNCTION()
	uint8 GetNoiseOctaves() const { return NoiseOctaves; }
	UFUNCTION()
	UBlocksDataAsset* GetBlocksData() const { return BlocksData; }

	UFUNCTION()
	int32 GetBlockSize() const { return BlockSize; }
	UFUNCTION()
	FVectorByte GetBlockCount() const { return BlockCount; }
	UFUNCTION()
	void GetBlockHeights(TArray<FBlockSpawnInfo>& OutHeights) const { OutHeights = BlockHeights; }
	
	UFUNCTION()
	FString GetWorldName() const { return WorldName; }

	UFUNCTION()
	float GetWorldSeed() const { return Seed; }

	UFUNCTION()
	FLinearColor GetTypeColor(const EBlockType& Type) const
	{
		if (!BlocksData || !BlocksData->Blocks.Contains(Type))
			return {};

		return BlocksData->Blocks[Type].Color;
	}

	UFUNCTION()
	bool GetBlockSafe(const FVector2DInt& ChunkIndex, const FVectorByte& BlockIndex, FBlock& Block) const;

	UFUNCTION(BlueprintPure)
	FVector2DInt WorldLocationToChunkIndex(const FVector& WorldLocation) const
	{
		return FVector2DInt{
			FMath::FloorToInt(WorldLocation.X / (BlockSize * BlockCount.X)),
			FMath::FloorToInt(WorldLocation.Y / (BlockSize * BlockCount.Y))
		};
	}

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise")
	float NoiseDamper;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise")
	uint8 NoiseOctaves;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise")
	float NoisePersistence;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blocks")
	UBlocksDataAsset* BlocksData;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blocks")
	TArray<FBlockSpawnInfo> BlockHeights;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blocks")
	int32 BlockSize = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blocks")
	FVectorByte BlockCount = {255, 255, 255};

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chunks")
	TSubclassOf<AChunk> ChunkBlueprint;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Chunks")
	int32 ChunkRenderDistance = 4;
	
	// Index is relative to player
	UPROPERTY(VisibleInstanceOnly, Category = "Chunks")
	TMap<FVector2DInt, AChunk*> Chunks;
	UPROPERTY(VisibleInstanceOnly, Category = "Chunks")
	TArray<AChunk*> ObsoleteChunks;

	UPROPERTY(VisibleInstanceOnly, Category = "Chunks")
	FVector2DInt LastPlayerIndex;
	UPROPERTY(EditAnywhere, Category = "Chunks")
	FString WorldName;
	UPROPERTY(EditAnywhere, Category = "Chunks")
	float Seed;
	

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USceneComponent* Root;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* HighlightCube;
	
	UPROPERTY()
	AMinecraftPlayerController* PC;

	UFUNCTION()
	void UpdateChunks(const FVector& PlayerPosition);
	UFUNCTION()
	void UpdatePlayer(const float& DeltaTime);

	UFUNCTION()
	void PlaceBlockAtPlayerSelection();

	// Could be on the PC as well, but since it only interacts with this actor anyway, there is no need.
	UPROPERTY()
	bool bPlayerIsMining; 
	UPROPERTY()
	float MiningTime;
	UPROPERTY()
	FVectorByte MiningBlockIndex;
	
	UFUNCTION()
	void StartBreakingBlocksAtPlayerSelection();
	UFUNCTION()
	void StopBreakingBlocksAtPlayerSelection();

	UPROPERTY()
	int32 CreatedChunkCount;
	
	UFUNCTION()
	void OnChunkCreated(const FVector2DInt& UpdatedIndex);
	UFUNCTION()
	void OnChunkUpdated(const FVector2DInt& UpdatedIndex);
	UFUNCTION()
	void OnChunkEdgeUpdated(const FVector2DInt& UpdatedIndex, const TArray<FVector2DInt>& Directions);
};
