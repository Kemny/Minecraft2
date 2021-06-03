#include "TerrainManager.h"

#include "Chunk.h"
#include "Kismet/GameplayStatics.h"

ATerrainManager::ATerrainManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATerrainManager::BeginPlay()
{
	Super::BeginPlay();

	for (int X = -ChunkRenderDistance; X <= ChunkRenderDistance; X++)
	{
		for (int Y = -ChunkRenderDistance; Y <= ChunkRenderDistance; Y++)
		{
			auto Spawned = GetWorld()->SpawnActor<AChunk>(ChunkBlueprint);
			
			FVector2DInt Index{X,Y};
			Spawned->InitializeVariables(this, Index);
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
		for (const auto & Chunk : Chunks)
			Chunk.Value->Rebuild(PlayerIndex);
		
		LastPlayerIndex = PlayerIndex;
	}
}


