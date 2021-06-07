#include "TerrainManager.h"

#include "Chunk.h"
#include "Kismet/GameplayStatics.h"
#include "MC2ElectricBoogaloo/Data/WorldSaveGame.h"
#include "MC2ElectricBoogaloo/Player/MinecraftPlayerController.h"

FVector ATerrainManager::ClampPlayerSelection(const FVector& SelectedPosition, const FVector& SelectedNormal, bool bSubtractNormal) const
{
	const float NormalMultiplier = (bSubtractNormal ? -1.f : 1.f) * (static_cast<float>(BlockSize) / 2);
	return FVector(
		FMath::FloorToInt((SelectedPosition.X + NormalMultiplier * SelectedNormal.X) / BlockSize),
		FMath::FloorToInt((SelectedPosition.Y + NormalMultiplier * SelectedNormal.Y) / BlockSize),
		FMath::FloorToInt((SelectedPosition.Z + NormalMultiplier * SelectedNormal.Z) / BlockSize)
	);
}

FVectorByte ATerrainManager::ClampedWorldLocationToBlockIndex(const FVector& Position) const
{
	const auto& BaseIndexX = static_cast<int32>(Position.X) % BlockCount.X;
	const auto& BaseIndexY = static_cast<int32>(Position.Y) % BlockCount.Y;
	const auto& BaseIndexZ = static_cast<int32>(Position.Z) % BlockCount.Z;
	
	return FVectorByte(
		BaseIndexX >= 0 ? BaseIndexX : BaseIndexX + BlockCount.X,
		BaseIndexY >= 0 ? BaseIndexY : BaseIndexY + BlockCount.Y,
		BaseIndexZ
	);
}

FVector ATerrainManager::ChunkIndexToWorldLocation(const FVector2DInt& ChunkIndex) const
{
	return FVector(
		ChunkIndex.X * BlockSize * BlockCount.X,
		ChunkIndex.Y * BlockSize * BlockCount.Y,
		0
	);
}

ATerrainManager::ATerrainManager()
{
	PrimaryActorTick.bCanEverTick = true;

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	if (!Root)
		return;
	
	SetRootComponent(Root);
	
	HighlightCube = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HighlightCube"));
	if (!HighlightCube)
		return;
	
	HighlightCube->SetupAttachment(Root);
}

void ATerrainManager::CreateTerrain(AMinecraftPlayerController* Player, const FString& SaveName, FTerrainDelegate OnGenerated)
{
	PC = Player;
	WorldName = SaveName;
	OnTerrainGenerated = OnGenerated;
	
	if (!PC || !HighlightCube)
		return;

	// Save is created in the menu, at this point there should be one
	if (!UWorldSaveGame::TryToLoadWorld(WorldName, Seed, StartPlayerPosition))
		return;

	// Bind Events
	PC->OnPlaceBlockRequest.AddUniqueDynamic(this, &ATerrainManager::PlaceBlockAtPlayerSelection);

	// Make the cube slightly larger than a regular block
	HighlightCube->SetWorldScale3D(FVector(BlockSize * 0.011));

	// Player Index
	LastPlayerIndex = WorldLocationToChunkIndex(StartPlayerPosition);

	// Set the player to the correct position when all the chunks are created
	CreatedChunkCount = 0;

	// Create Chunks and populate blocks
	for (int X = -ChunkRenderDistance; X <= ChunkRenderDistance; X++)
	{
		for (int Y = -ChunkRenderDistance; Y <= ChunkRenderDistance; Y++)
		{
			auto Spawned = GetWorld()->SpawnActor<AChunk>(ChunkBlueprint);
			
			const auto Index = FVector2DInt{X,Y} + LastPlayerIndex;
			
			Spawned->InitializeVariables(this);
			Spawned->OnUpdated.AddUniqueDynamic(this, &ATerrainManager::OnChunkCreated);
			Spawned->OnUpdated.AddUniqueDynamic(this, &ATerrainManager::OnChunkUpdated);
			Spawned->OnEdgeUpdated.BindDynamic(this, &ATerrainManager::OnChunkEdgeUpdated);
			Spawned->BuildBlocks(Index);
			Spawned->SetActorLocation(ChunkIndexToWorldLocation(Index));

			Chunks.Add(Index, Spawned);
		}
	}

	// Split from above so geometry doesn't need to update multiple times
	for (const auto & Chunk : Chunks)
	{
		Chunk.Value->RebuildGeometry();
	}
}

bool ATerrainManager::GetBlockSafe(const FVector2DInt& ChunkIndex, const FVectorByte& BlockIndex, FBlock& Block) const
{
	return Chunks.Contains(ChunkIndex) && Chunks[ChunkIndex]->GetBlockSafe(BlockIndex, Block);
}

void ATerrainManager::PlaceBlockAtPlayerSelection()
{
	if (!PC)
		return;

	const auto& SelectedPosition = PC->GetSelectedPosition();
	const auto& BaseBlockVector = ClampPlayerSelection(SelectedPosition.Position, SelectedPosition.Normal, false);

	// Don't place blocks inside player
	FHitResult Hit;
	const FVector TracePos = BaseBlockVector * BlockSize - FVector(-1 * BlockSize / 2);
	const bool HitPlayer = GetWorld()->SweepSingleByObjectType(
		Hit,
		TracePos,
		TracePos,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeBox(HighlightCube->Bounds.BoxExtent)
	);
	
	const auto& CurrentBlockIndex = ClampedWorldLocationToBlockIndex(BaseBlockVector);
	const auto& ChunkIndex = WorldLocationToChunkIndex(SelectedPosition.Position);
	if (!HitPlayer && Chunks.Contains(ChunkIndex) && Chunks[ChunkIndex]->ContainsBlock(CurrentBlockIndex))
	{
		Chunks[ChunkIndex]->AddBlock(CurrentBlockIndex, PC->GetSelectedBlock());
	}
}

void ATerrainManager::OnChunkCreated(const FVector2DInt& UpdatedIndex)
{
	CreatedChunkCount++;
	Chunks[UpdatedIndex]->OnUpdated.RemoveDynamic(this, &ATerrainManager::OnChunkCreated);

	// If all initial chunks were created, let the game spawn player, etc.
	if (CreatedChunkCount == Chunks.Num())
	{
		// ReSharper disable once CppExpressionWithoutSideEffects
		OnTerrainGenerated.ExecuteIfBound();
	}
}

void ATerrainManager::OnChunkUpdated(const FVector2DInt& UpdatedIndex)
{
	Chunks[UpdatedIndex]->SetActorLocation(ChunkIndexToWorldLocation(UpdatedIndex));

	// If a chunk is updated, check if any other chunks are waiting for block information
	for (const auto & Chunk : Chunks)
	{
		TArray<FVector2DInt> Directions;
		Chunk.Value->GetMissingChunkDirections(Directions);
		for (const auto & Direction : Directions)
		{
			if (Chunk.Key + Direction == UpdatedIndex)
			{
				Chunk.Value->RebuildGeometry();
				break;
			}
		}
	}
}

void ATerrainManager::OnChunkEdgeUpdated(const FVector2DInt& UpdatedIndex, const TArray<FVector2DInt>& Directions)
{
	// Update Chunks only next to the updated chunk in the correct direction
	for (const auto & Direction : Directions)
	{
		const auto TargetIndex = UpdatedIndex + Direction;
		if (Chunks.Contains(TargetIndex))
			Chunks[TargetIndex]->RebuildGeometry();
	}
}

void ATerrainManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (CreatedChunkCount != Chunks.Num())
		return;
	
	// If there is no PC, there is no need to continue with the tick
	if (!PC)
		return;

	// Null safety
	if (const auto Pawn = PC->GetPawn())
	{
		const auto& PlayerPosition = Pawn->GetActorLocation();

		UpdateChunks(PlayerPosition);
		UpdatePlayer(DeltaTime);
	}
}

void ATerrainManager::UpdateChunks(const FVector& PlayerPosition)
{
	const FVector2DInt& PlayerIndex = WorldLocationToChunkIndex(PlayerPosition);

	// Check if new chunks need to be rendered
	if (PlayerIndex != LastPlayerIndex)
	{
		const auto XMax = PlayerIndex.X + ChunkRenderDistance;
		const auto XMin = PlayerIndex.X - ChunkRenderDistance;
		const auto YMax = PlayerIndex.Y + ChunkRenderDistance;
		const auto YMin = PlayerIndex.Y - ChunkRenderDistance;

		for (auto & Chunk : Chunks)
		{
			// If the chunk is too far away from the player, make it obsolete
			const auto& WP = Chunk.Value->GetWorldIndex();
			if (WP.X > XMax || WP.X < XMin || WP.Y > YMax || WP.Y < YMin)
			{
				ObsoleteChunks.Emplace(Chunk.Value);
			}
		}

		// Remove obsolete from active chunks. Done outside of the first loop for safety
		for (const auto& ObsoleteChunk : ObsoleteChunks)
		{
			Chunks.Remove(ObsoleteChunk->GetWorldIndex());
		}

		// Keep track of which obsolete chunks were already used
		int32 ObsoleteChunkIndex = 0;
		// Generate Block Data
		for (int32 X = -ChunkRenderDistance; X <= ChunkRenderDistance; X++)
		{
			for (int32 Y = -ChunkRenderDistance; Y <= ChunkRenderDistance; Y++)
			{
				const FVector2DInt Index{X + PlayerIndex.X,Y + PlayerIndex.Y};
				
				// If block is within player range, but isn't rendered
				if (!Chunks.Contains(Index))
				{
					const auto ChunkNum = ObsoleteChunks.Num();
					if (ChunkNum == 0)
					{
						UE_LOG(LogTemp, Error, TEXT("Failed to Deque a Chunk for position: [%i, %i]"), X, Y);
						continue;
					}
					AChunk* Chunk = ObsoleteChunks[ObsoleteChunkIndex];
					ObsoleteChunkIndex++;
					
					Chunks.Add(Index, Chunk);
					// Rebuild Blocks for the chunk
					Chunk->BuildBlocks(Index);
				}
			}
		}

		// Now that blocks are rebuild, geometry can be safely rebuilt as well
		for (const auto & ObsoleteChunk : ObsoleteChunks)
		{
			ObsoleteChunk->RebuildGeometry();
		}
		ObsoleteChunks.Empty();

		LastPlayerIndex = PlayerIndex;
	}
}

void ATerrainManager::UpdatePlayer(const float& DeltaTime)
{
	if (PC->GetCurrentState() == EPlayerState::Pickaxe)
	{
		const auto& SelectedPosition = PC->GetSelectedPosition();
		const FVector BaseBlockVector = ClampPlayerSelection(SelectedPosition.Position, SelectedPosition.Normal, true);
		
		if (BaseBlockVector.Z < BlockCount.Z)
			HighlightCube->SetWorldLocation(BaseBlockVector * BlockSize);

		if (PC->IsMining())
		{
			// Update For how long the player has been mining a block
			PC->UpdateMiningTime(DeltaTime);

			const auto& CurrentBlockIndex = ClampedWorldLocationToBlockIndex(BaseBlockVector);

			// If player started mining another block
			if (CurrentBlockIndex != PC->GetCurrentMiningBlock())
				PC->SetCurrentMiningBlock(CurrentBlockIndex);
			
			const auto& ChunkIndex = WorldLocationToChunkIndex(SelectedPosition.Position);
			EBlockType BlockType;
			
			// Chunk and Block Exists
			if (Chunks.Contains(ChunkIndex) && Chunks[ChunkIndex]->GetBlockTypeSafe(PC->GetCurrentMiningBlock(), BlockType))
			{
				const auto& Data = BlocksData->Blocks[BlockType];
				// Has the player been mining for long enough?
				if (Data.bIsBreakable && Data.BreakTime <= PC->GetMiningTime())
				{
					Chunks[ChunkIndex]->RemoveBlock(PC->GetCurrentMiningBlock());
					PC->ResetMiningTime();
				}
			}
			
		}
	}
	else if (PC->GetCurrentState() == EPlayerState::PlaceBlock)
	{
		const auto& SelectedPosition = PC->GetSelectedPosition();
		const FVector BaseBlockVector = ClampPlayerSelection(SelectedPosition.Position, SelectedPosition.Normal, false);
		HighlightCube->SetWorldLocation(BaseBlockVector * BlockSize);
	}
	else
	{
		HighlightCube->SetWorldLocation({0,0,-100});
	}
}

void ATerrainManager::DestroyTerrain()
{
	// Split from above so geometry doesn't need to update multiple times
	for (auto & Chunk : Chunks)
	{
		Chunk.Value->TryToSave();
		Chunk.Value->Destroy();
	}
	Chunks.Empty();
}