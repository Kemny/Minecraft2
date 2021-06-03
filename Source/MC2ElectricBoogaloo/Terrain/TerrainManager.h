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
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	int32 GetBlockSize() const { return BlockSize; }
	FVectorByte GetBlockCount() const { return BlockCount; }
	void GetBlockHeights(TArray<FBlockSpawnInfo>& OutHeights) const { OutHeights = BlockHeights; }
	
	FLinearColor GetTypeColor(const EBlockType& Type) const
	{
		if (BlockColors.Contains(Type))
			return BlockColors[Type];
		
		return {};
	}
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AChunk> ChunkBlueprint;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TMap<EBlockType, FLinearColor> BlockColors;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FBlockSpawnInfo> BlockHeights;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 BlockSize = 1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVectorByte BlockCount = {255, 255, 255};

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ChunkRenderDistance = 4;
	
	// Index is relative to player
	UPROPERTY()
	TMap<FVector2DInt, AChunk*> Chunks;

	UPROPERTY()
	FVector2DInt LastPlayerIndex;

	UPROPERTY(VisibleAnywhere)
	AChunk* C1;
	UPROPERTY(VisibleAnywhere)
	AChunk* C2;
};
