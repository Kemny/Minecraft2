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
	
	if (!PC || !HighlightCube)
		return;
	
	PC->OnPlaceBlockRequest.AddUniqueDynamic(this, &ATerrainManager::PlaceBlockAtPlayerSelection);
	PC->OnStartMining.AddUniqueDynamic(this, &ATerrainManager::StartBreakingBlocksAtPlayerSelection);
	PC->OnStopMining.AddUniqueDynamic(this, &ATerrainManager::StopBreakingBlocksAtPlayerSelection);
	
	
	HighlightCube->SetWorldScale3D(FVector(BlockSize * 0.011));
	CreatedChunkCount = 0;

	// Create Chunks and populate blocks
	for (int X = -ChunkRenderDistance; X <= ChunkRenderDistance; X++)
	{
		for (int Y = -ChunkRenderDistance; Y <= ChunkRenderDistance; Y++)
		{
			auto Spawned = GetWorld()->SpawnActor<AChunk>(ChunkBlueprint);
			
			FVector2DInt Index{X,Y};
			Spawned->InitializeVariables(this);
			Spawned->OnUpdated.AddUniqueDynamic(this, &ATerrainManager::OnChunkCreated);
			Spawned->OnUpdated.AddUniqueDynamic(this, &ATerrainManager::OnChunkUpdated);
			Spawned->OnEdgeUpdated.BindDynamic(this, &ATerrainManager::OnChunkEdgeUpdated);
			Spawned->RebuildBlocks(Index);

			const auto NewPosition = FVector(
				X * BlockSize * BlockCount.X,
				Y * BlockSize * BlockCount.Y,
				0
			);
			Spawned->SetActorLocation(NewPosition);

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
	// TODO Clean up all code related to pc
	
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

	//const FName TraceTag("MyTraceTag");
	//GetWorld()->DebugDrawTraceTag = TraceTag;
	
	FCollisionQueryParams Params;
	//Params.TraceTag = TraceTag;

	FVector TracePos = BaseBlockVector * BlockSize - FVector(-1 * BlockSize / 2);
	FHitResult Hit;
	const bool HitPlayer = GetWorld()->SweepSingleByObjectType(
		Hit,
		TracePos,
		TracePos,
		FQuat::Identity,
		ECC_Pawn,
		FCollisionShape::MakeBox(HighlightCube->Bounds.BoxExtent),
		Params
	);
	
	// TODO Player Collision
	if (!HitPlayer && Chunks.Contains(ChunkIndex) && Chunks[ChunkIndex]->ContainsBlock(CurrentBlockIndex))
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

void ATerrainManager::OnChunkCreated(const FVector2DInt& UpdatedIndex)
{
	CreatedChunkCount++;
	Chunks[UpdatedIndex]->OnUpdated.RemoveDynamic(this, &ATerrainManager::OnChunkCreated);
	
	if (CreatedChunkCount == Chunks.Num())
		OnNewTerrainGenerated.Broadcast();
}

void ATerrainManager::OnChunkUpdated(const FVector2DInt& UpdatedIndex)
{
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

	if (!PC)
		return;

	const auto Pawn = PC->GetPawn();
	if (!Pawn)
		return;

	const auto& PlayerPosition = Pawn->GetActorLocation();

	UpdateChunks(PlayerPosition);

	UpdatePlayer(DeltaTime);
}

void ATerrainManager::UpdateChunks(const FVector& PlayerPosition)
{
	const FVector2DInt& PlayerIndex = WorldLocationToChunkIndex(PlayerPosition);

	if (PlayerIndex != LastPlayerIndex)
	{
		// TODO Clean up Implementation
		const auto XMax = PlayerIndex.X + ChunkRenderDistance;
		const auto XMin = PlayerIndex.X - ChunkRenderDistance;
		
		const auto YMax = PlayerIndex.Y + ChunkRenderDistance;
		const auto YMin = PlayerIndex.Y - ChunkRenderDistance;

		for (auto & Chunk : Chunks)
		{
			const auto& WP = Chunk.Value->GetWorldIndex();
			if (WP.X > XMax || WP.X < XMin || WP.Y > YMax || WP.Y < YMin)
			{
				ObsoleteChunks.Emplace(Chunk.Value);
			}
		}

		// Remove obsolete from active chunks
		for (const auto& ObsoleteChunk : ObsoleteChunks)
		{
			Chunks.Remove(ObsoleteChunk->GetWorldIndex());
		}

		int32 ObsoleteChunkIndex = 0;
		// TODO Chunk Creation Events for correct Blocks
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
					AChunk* Chunk = ObsoleteChunks[ObsoleteChunkIndex];
					ObsoleteChunkIndex++;
					
					Chunks.Add(Index, Chunk);
					Chunk->RebuildBlocks(Index);
				}
			}
		}

		for (const auto & ObsoleteChunk : ObsoleteChunks)
		{
			ObsoleteChunk->RebuildGeometry();
			
			const auto NewPosition = FVector(
				ObsoleteChunk->GetWorldIndex().X * BlockSize * BlockCount.X,
				ObsoleteChunk->GetWorldIndex().Y * BlockSize * BlockCount.Y,
				0
			);
	
			ObsoleteChunk->SetActorLocation(NewPosition);
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