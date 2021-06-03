#include "TerrainManager.h"

#include "Chunk.h"
#include "Kismet/GameplayStatics.h"

ATerrainManager::ATerrainManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATerrainManager::CreateTerrain()
{
	for (int X = -ChunkRenderDistance; X <= ChunkRenderDistance; X++)
	{
		for (int Y = -ChunkRenderDistance; Y <= ChunkRenderDistance; Y++)
		{
			auto Spawned = GetWorld()->SpawnActor<AChunk>(ChunkBlueprint);
			
			FVector2DInt Index{X,Y};
			Spawned->InitializeVariables(this, {X,Y});
			Chunks.Add(Index, Spawned);
		}
	}
}

void ATerrainManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const auto World = GetWorld();
	
	if (!World)
		return;
	
	// Player Controller and pawn aren't cached to allow simple destruction. Can be changed for performance
	const auto PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
		return;
	
	const auto Pawn = UGameplayStatics::GetPlayerPawn(World, 0);
	if (!Pawn)
		return;

	const auto& PlayerPosition = Pawn->GetActorLocation();

	// Auto Clamped to int
	const FVector2DInt& PlayerIndex{FMath::FloorToInt(PlayerPosition.X / (BlockSize * BlockCount.X)), FMath::FloorToInt(PlayerPosition.Y / (BlockSize * BlockCount.Y))};

	if (PlayerIndex != LastPlayerIndex)
	{
		// TODO Clean up Implementation
		const auto XMax = PlayerIndex.X + ChunkRenderDistance;
		const auto XMin = PlayerIndex.X - ChunkRenderDistance;
		
		const auto YMax = PlayerIndex.Y + ChunkRenderDistance;
		const auto YMin = PlayerIndex.Y - ChunkRenderDistance;

		TArray<AChunk*> ObsoleteChunks;
		for (auto & Chunk : Chunks)
		{
			const auto& WP = Chunk.Value->GetWorldPosition();
			if (WP.X > XMax || WP.X < XMin || WP.Y > YMax || WP.Y < YMin)
			{
				ObsoleteChunks.Emplace(Chunk.Value);
			}
		}

		for (const auto & ObsoleteChunk : ObsoleteChunks)
			Chunks.Remove(ObsoleteChunk->GetWorldPosition());
		
		for (int32 X = -ChunkRenderDistance; X <= ChunkRenderDistance; X++)
		{
			for (int32 Y = -ChunkRenderDistance; Y <= ChunkRenderDistance; Y++)
			{
				const FVector2DInt Index{X + PlayerIndex.X,Y + PlayerIndex.Y};
				if (!Chunks.Contains(Index))
				{
					const auto ChunkNum = ObsoleteChunks.Num();
					if (ChunkNum == 0)
					{
						UE_LOG(LogTemp, Error, TEXT("Failed to Deque a Chunk for position: [%i, %i]"), X, Y);
						continue;
					}
					AChunk* Chunk = ObsoleteChunks[ChunkNum - 1];
					ObsoleteChunks.RemoveAt(ChunkNum - 1);
					
					Chunks.Add(Index, Chunk);
					Chunk->Rebuild(Index);
				}
			}
		}

		LastPlayerIndex = PlayerIndex;
	}
}




