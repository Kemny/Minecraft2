#include "TerrainManager.h"

#include "Chunk.h"
#include "Kismet/GameplayStatics.h"
#include "MC2ElectricBoogaloo/Player/MinecraftPlayerController.h"

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

void ATerrainManager::CreateTerrain()
{
	PC = GetWorld()->GetFirstPlayerController<AMinecraftPlayerController>();
	PC->OnPlaceBlockRequest.AddUniqueDynamic(this, &ATerrainManager::PlaceBlockAtPlayerSelection);
	PC->OnStartMining.AddUniqueDynamic(this, &ATerrainManager::StartBreakingBlocksAtPlayerSelection);
	PC->OnStopMining.AddUniqueDynamic(this, &ATerrainManager::StopBreakingBlocksAtPlayerSelection);
	
	if (!PC || !HighlightCube)
		return;
	
	HighlightCube->SetWorldScale3D(FVector(BlockSize * 0.011));
	
	for (int X = -ChunkRenderDistance; X <= ChunkRenderDistance; X++)
	{
		for (int Y = -ChunkRenderDistance; Y <= ChunkRenderDistance; Y++)
		{
			auto Spawned = GetWorld()->SpawnActor<AChunk>(ChunkBlueprint);
			
			FVector2DInt Index{X,Y};
			Spawned->InitializeVariables(this);
			Spawned->Rebuild({X,Y});
			Chunks.Add(Index, Spawned);
		}
	}
	
	OnNewTerrainGenerated.Broadcast();
}

EBlockType ATerrainManager::GetChunkBlockType(const FVector2DInt& ChunkIndex, const FVectorByte& BlockIndex)
{
	FBlock Block;
	
	if (Chunks.Contains(ChunkIndex) && Chunks[ChunkIndex]->GetBlockSafe(BlockIndex, Block))
		return Block.Type;
	
	return EBlockType::End;
}

void ATerrainManager::PlaceBlockAtPlayerSelection()
{
	// TODO Clean up all everything related to pc
	
	if (!PC)
		return;

	const auto& SelectedPosition = PC->GetSelectedPosition();
	const FVector BaseBlockVector(
		FMath::FloorToInt((SelectedPosition.Position.X + SelectedPosition.Normal.X * (BlockSize / 2)) / BlockSize),
		FMath::FloorToInt((SelectedPosition.Position.Y + SelectedPosition.Normal.Y * (BlockSize / 2)) / BlockSize),
		FMath::FloorToInt((SelectedPosition.Position.Z + SelectedPosition.Normal.Z * (BlockSize / 2)) / BlockSize)
	);
	
	const auto& BaseIndexX = static_cast<int32>(BaseBlockVector.X) % BlockCount.X;
	const auto& BaseIndexY = static_cast<int32>(BaseBlockVector.Y) % BlockCount.Y;
	const auto& BaseIndexZ = static_cast<int32>(BaseBlockVector.Z) % BlockCount.Z;
			
	const FVectorByte CurrentBlockIndex(
		(BaseIndexX >= 0 ? BaseIndexX : BaseIndexX + BlockCount.X),
		(BaseIndexY >= 0 ? BaseIndexY : BaseIndexY + BlockCount.Y),
		BaseIndexZ
	);

	const auto& ChunkIndex = WorldLocationToChunkIndex(SelectedPosition.Position);
	if (Chunks.Contains(ChunkIndex) && Chunks[ChunkIndex]->ContainsBlock(CurrentBlockIndex))
	{
		Chunks[ChunkIndex]->AddBlock(CurrentBlockIndex, PC->GetSelectedBlock());
	}
}

void ATerrainManager::StartBreakingBlocksAtPlayerSelection()
{
	MiningTime = 0;
	bPlayerIsMining = true;
}

void ATerrainManager::StopBreakingBlocksAtPlayerSelection()
{
	bPlayerIsMining = false;
}

void ATerrainManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const auto World = GetWorld();
	
	if (!World)
		return;
	
	if (!PC)
		return;
	
	const auto Pawn = UGameplayStatics::GetPlayerPawn(World, 0);
	if (!Pawn)
		return;

	const auto& PlayerPosition = Pawn->GetActorLocation();

	// Auto Clamped to int
	const FVector2DInt& PlayerIndex = WorldLocationToChunkIndex(PlayerPosition);

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

		// Generate Block Data
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
					Chunk->RebuildBlocks(Index);
				}
			}
		}
		// TODO Don't rebuild everything
		// Generate Geometry
		for (int32 X = -ChunkRenderDistance; X <= ChunkRenderDistance; X++)
		{
			for (int32 Y = -ChunkRenderDistance; Y <= ChunkRenderDistance; Y++)
			{
				const FVector2DInt Index{X + PlayerIndex.X,Y + PlayerIndex.Y};
				Chunks[Index]->RebuildGeometry();
			}
		}

		LastPlayerIndex = PlayerIndex;
	}

	if (PC->GetCurrentState() == EPlayerState::Pickaxe)
	{
		const auto& SelectedPosition = PC->GetSelectedPosition();
		const FVector BaseBlockVector(
			FMath::FloorToInt((SelectedPosition.Position.X - SelectedPosition.Normal.X * (BlockSize / 2)) / BlockSize),
			FMath::FloorToInt((SelectedPosition.Position.Y - SelectedPosition.Normal.Y * (BlockSize / 2)) / BlockSize),
			FMath::FloorToInt((SelectedPosition.Position.Z - SelectedPosition.Normal.Z * (BlockSize / 2)) / BlockSize)
		);
		HighlightCube->SetWorldLocation(BaseBlockVector * BlockSize);

		if (bPlayerIsMining)
		{
			MiningTime += DeltaTime;

			

			const auto& BaseIndexX = static_cast<int32>(BaseBlockVector.X) % BlockCount.X;
			const auto& BaseIndexY = static_cast<int32>(BaseBlockVector.Y) % BlockCount.Y;
			const auto& BaseIndexZ = static_cast<int32>(BaseBlockVector.Z) % BlockCount.Z;
			
			const FVectorByte CurrentBlockIndex(
				(BaseIndexX >= 0 ? BaseIndexX : BaseIndexX + BlockCount.X),
				(BaseIndexY >= 0 ? BaseIndexY : BaseIndexY + BlockCount.Y),
				BaseIndexZ
			);
			
			if (CurrentBlockIndex != MiningBlockIndex)
			{
				MiningTime = 0;
				MiningBlockIndex = CurrentBlockIndex;
			}
			
			const auto& ChunkIndex = WorldLocationToChunkIndex(SelectedPosition.Position);
			EBlockType BlockType;
			// Block Exists
			if (Chunks.Contains(ChunkIndex) && Chunks[ChunkIndex]->GetBlockTypeSafe(MiningBlockIndex, BlockType))
			{
				const auto& Data = BlocksData->Blocks[BlockType];
				if (Data.bIsBreakable && Data.BreakTime <= MiningTime)
				{
					MiningTime = 0;
					Chunks[ChunkIndex]->RemoveBlock(MiningBlockIndex);
				}
			}
			
		}
	}
	else if (PC->GetCurrentState() == EPlayerState::PlaceBlock)
	{
		const auto& SelectedPosition = PC->GetSelectedPosition();
		const FVector BaseBlockVector(
			FMath::FloorToInt((SelectedPosition.Position.X + SelectedPosition.Normal.X * (BlockSize / 2)) / BlockSize),
			FMath::FloorToInt((SelectedPosition.Position.Y + SelectedPosition.Normal.Y * (BlockSize / 2)) / BlockSize),
			FMath::FloorToInt((SelectedPosition.Position.Z + SelectedPosition.Normal.Z * (BlockSize / 2)) / BlockSize)
		);
		HighlightCube->SetWorldLocation(BaseBlockVector * BlockSize);
	}
	else
	{
		HighlightCube->SetWorldLocation({0,0,-100});
	}
}




