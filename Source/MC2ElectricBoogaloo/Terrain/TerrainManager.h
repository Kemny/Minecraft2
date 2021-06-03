#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "MC2ElectricBoogaloo/Data/BlockData.h"

#include "TerrainManager.generated.h"

#define INDEX(x,y,SizeY) x * SizeY + y

class AChunk;

USTRUCT(BlueprintType)
struct FBlockSpawnInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	EBlockType Type;
	UPROPERTY(EditAnywhere)
	uint8 SpawnStopHeight;
};

UCLASS(Blueprintable)
class MC2ELECTRICBOOGALOO_API ATerrainManager : public AActor
{
	GENERATED_BODY()

public:	
	ATerrainManager();
	virtual void Tick(float DeltaTime) override;
	
	float GetNoisePersistence() const { return NoisePersistence; }
	float GetNoiseDamper() const { return NoiseDamper; }
	uint8 GetNoiseOctaves() const { return NoiseOctaves; }
	
	int32 GetBlockSize() const { return BlockSize; }
	FVectorByte GetBlockCount() const { return BlockCount; }
	void GetBlockHeights(TArray<FBlockSpawnInfo>& OutHeights) const { OutHeights = BlockHeights; }
	
	FLinearColor GetTypeColor(const EBlockType& Type) const
	{
		if (BlockColors.Contains(Type))
			return BlockColors[Type];
		
		return {};
	}
	
	UFUNCTION(BlueprintCallable)
	void CreateTerrain();
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise")
	float NoiseDamper;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise")
	uint8 NoiseOctaves;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Noise")
	float NoisePersistence;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Blocks")
	TMap<EBlockType, FLinearColor> BlockColors;
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
	UPROPERTY(VisibleAnywhere, Category = "Chunks")
	TMap<FVector2DInt, AChunk*> Chunks;

	UPROPERTY(VisibleAnywhere, Category = "Chunks")
	FVector2DInt LastPlayerIndex;
};
