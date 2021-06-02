#include "TerrainManager.h"

#include "Kismet/GameplayStatics.h"
#include "ProceduralMeshComponent.h"
#include "Kismet/KismetStringLibrary.h"


ATerrainManager::ATerrainManager()
{
	PrimaryActorTick.bCanEverTick = true;
}

void ATerrainManager::CreateTriangle(UProceduralMeshComponent* Mesh)
{
	TArray<FVector> Vertices;
	Vertices.Add(FVector(0, 0, 0));
	Vertices.Add(FVector(0, 100, 0));
	Vertices.Add(FVector(0, 0, 100));
	Vertices.Add(FVector(0, 100, 100));

	TArray<int32> Triangles;
	Triangles.Add(0);
	Triangles.Add(1);
	Triangles.Add(2);
	
	Triangles.Add(1);
	Triangles.Add(3);
	Triangles.Add(2);

	TArray<FVector> Normals;
	Normals.Add(FVector(1, 0, 0));
	Normals.Add(FVector(1, 0, 0));
	Normals.Add(FVector(1, 0, 0));
	Normals.Add(FVector(1, 0, 0));
	Normals.Add(FVector(1, 0, 0));
	Normals.Add(FVector(1, 0, 0));

	TArray<FVector2D> UVs;
	UVs.Add({0,0});
	UVs.Add({0,1});
	UVs.Add({1,0});
	UVs.Add({1,1});

	TArray<FLinearColor> VertexColors;
	VertexColors.Add(FLinearColor(1, 1, 1, 1.0));
	VertexColors.Add(FLinearColor(1, 0, 0, 1.0));
	VertexColors.Add(FLinearColor(0, 0, 1, 1.0));
	VertexColors.Add(FLinearColor(1, 1, 1, 1.0));

	Mesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, UVs, VertexColors, TArray<FProcMeshTangent>{}, true);
        
	// Enable collision data
	Mesh->ContainsPhysicsTriMeshData(true);
}

void ATerrainManager::RebuildChunk(FChunk& Chunk, const FVector& WorldPosition)
{
	Chunk.Mesh->SetWorldLocation(WorldPosition);
	Chunk.Blocks.Empty();
	
	for (uint8 X = 0; X < ChunkBlockSize.X; ++X)
	{
		for (uint8 Y = 0; Y < ChunkBlockSize.Y; ++Y)
		{
			for (uint8 Z = 0; Z < ChunkBlockSize.Z; ++Z)
			{
				if (Z != 0)
				{
					Chunk.Blocks.Add(FVectorByte{X,Y,Z}, {EBlockType::Air});
				}
				else
				{
					Chunk.Blocks.Add(FVectorByte{X, Y,0}, {EBlockType::Stone});
				}
			}
		}
	}
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FLinearColor> VertexColors;
	int32 Triangle = 0;

	// Probably not the best, but it works well enough
	// Go over each height
	for (uint8 Z = 0; Z < ChunkBlockSize.Z; Z++)
	{
		// Go over each block direction
		for (int32 D = 0; D < 1; D++)
		{
			const auto Direction = static_cast<EBlockDirection>(D);

			FVector* BottomLeft = nullptr;
			FVector* TopLeft = nullptr;
			FVector* BottomRight = nullptr;
			
			// Go over each block on this layer
			for (uint8 X = 0; X < ChunkBlockSize.X; X++)
			{
				for (uint8 Y = 0; Y < ChunkBlockSize.Y; Y++)
				{
					const auto Index = FVectorByte(X,Y,Z);
					auto& Block = Chunk.Blocks[Index];
					const auto PossibleAirBlockIndex = BlockDirectionVector(Direction) + Index;

					// Check if there is a block next to it
					FBlock PossibleAirBlock;
					if (!GetBlockSafe(Chunk, PossibleAirBlockIndex, PossibleAirBlock))
						continue;
					
					// Check if calculation is necessary
					if (!IsBlockTransparent(Block.Type) && IsBlockTransparent(PossibleAirBlock.Type))
					{
						int32 NewIndex = INDEX_NONE;
						// Not uint8 since it would just wrap back
						for (int16 i = Y - 1; i >= 0; i--)
						{
							NewIndex = Vertices.IndexOfByKey(FVector(X * BlockSize, i * BlockSize, Z * BlockSize));
							
							if (NewIndex != INDEX_NONE)
								break;
						}
						
						if (NewIndex == INDEX_NONE)
						{
							for (int16 i = X - 1; i >= 0; i--)
							{
								//TODO Found index never used
								NewIndex = Vertices.IndexOfByKey(FVector(i * BlockSize, FoundIndex * BlockSize, Z * BlockSize));
							
								if (NewIndex != INDEX_NONE)
									break;
							}
							
							if (NewIndex == INDEX_NONE)
							{
								const auto Position = FVector(X * BlockSize, Y * BlockSize, Z * BlockSize);
								Vertices.Add(Position);
								Vertices.Add(Position); // Placeholder for triangles

								//Vertices.Add(FVector((X + 1) * BlockSize, Y * BlockSize, Z * BlockSize));
							
								Triangles.Add(Triangle + 1);
								Triangles.Add(Triangle);
								Triangle += 2;
							}
						}
						
						NewIndex = INDEX_NONE;
						if (!HasBlock(Chunk, FVectorByte(X, Y + 1,Z)))
						{
							for (int16 i = X - 1; i >= 0; i--)
							{
								NewIndex = Vertices.IndexOfByKey(FVector(i * BlockSize, (Y + 1) * BlockSize, Z * BlockSize));
							
								if (NewIndex != INDEX_NONE)
									break;
							}
							if (NewIndex == INDEX_NONE)
							{
								Vertices.Add(FVector(X * BlockSize, (Y + 1) * BlockSize, Z * BlockSize));
								Vertices.Add(FVector((X + 1) * BlockSize, (Y + 1) * BlockSize, Z * BlockSize));

								Triangles.Add(Triangle);
								Triangles.Add(Triangle - 1);
								Triangles.Add(Triangle);
								Triangles.Add(Triangle + 1);
								
								Triangle += 2;
							}
							if (true)
							{
								
							}
							Vertices.Add(FVector(X * BlockSize, (Y + 1) * BlockSize, Z * BlockSize));
							Vertices.Add(FVector((X + 1) * BlockSize, (Y + 1) * BlockSize, Z * BlockSize));

							Triangles.Add(Triangle);
							Triangles.Add(Triangle - 1);
							Triangles.Add(Triangle);
							Triangles.Add(Triangle + 1);
								
							Triangle += 2;
							
							Normals.Add(FVector(1, 0, 0));
							Normals.Add(FVector(1, 0, 0));
							Normals.Add(FVector(1, 0, 0));
							Normals.Add(FVector(1, 0, 0));
							Normals.Add(FVector(1, 0, 0));
							Normals.Add(FVector(1, 0, 0));
			
							VertexColors.Add(BlockColors[Block.Type]);
							VertexColors.Add(BlockColors[Block.Type]);
							VertexColors.Add(BlockColors[Block.Type]);
							VertexColors.Add(BlockColors[Block.Type]);
							VertexColors.Add(BlockColors[Block.Type]);
							VertexColors.Add(BlockColors[Block.Type]);
						}
					}
					else
					{
						
					}
				}
			}
		}
	}
	UE_LOG(LogTemp, Warning, TEXT("Triangles: %i"), Triangles.Num());
	UE_LOG(LogTemp, Warning, TEXT("Vertices: %i"), Vertices.Num());
	
	Chunk.Mesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, TArray<FVector2D>{}, VertexColors, TArray<FProcMeshTangent>{}, true);
	Chunk.Mesh->ContainsPhysicsTriMeshData(true);

	/*
	TArray<FVector> Vertices;
	TArray<int32> Triangles;
	TArray<FVector> Normals;
	TArray<FLinearColor> VertexColors;
	int32 triangle = 0;
	for (const auto & Block : Chunk.Blocks)
	{
		if (Block.Value.Type == EBlockType::Air)
			continue;
		
		if (Chunk.Blocks.Contains(Block.Key + FVectorByte{0,0,1}) && Chunk.Blocks[Block.Key + FVectorByte{0,0,1}].Type == EBlockType::Air)
		{
			Vertices.Add(FVector(Block.Key.X * BlockSize, Block.Key.Y * BlockSize, Block.Key.Z * BlockSize));
			Vertices.Add(FVector((Block.Key.X + 1) * BlockSize, Block.Key.Y * BlockSize, Block.Key.Z));
			Vertices.Add(FVector(Block.Key.X * BlockSize, (Block.Key.Y + 1) * BlockSize, Block.Key.Z * BlockSize));
			Vertices.Add(FVector((Block.Key.X + 1) * BlockSize, (Block.Key.Y + 1) * BlockSize, Block.Key.Z * BlockSize));

			Triangles.Add(triangle + 1);
			Triangles.Add(triangle);
			Triangles.Add(triangle + 2);
			Triangles.Add(triangle + 1);
			Triangles.Add(triangle + 2);
			Triangles.Add(triangle + 3);
			
			Normals.Add(FVector(1, 0, 0));
			Normals.Add(FVector(1, 0, 0));
			Normals.Add(FVector(1, 0, 0));
			Normals.Add(FVector(1, 0, 0));
			Normals.Add(FVector(1, 0, 0));
			Normals.Add(FVector(1, 0, 0));
			
			VertexColors.Add(BlockColors[Block.Value.Type]);
			VertexColors.Add(BlockColors[Block.Value.Type]);
			VertexColors.Add(BlockColors[Block.Value.Type]);
			VertexColors.Add(BlockColors[Block.Value.Type]);

			triangle += 4;
		}
	}
	Chunk.Mesh->CreateMeshSection_LinearColor(0, Vertices, Triangles, Normals, TArray<FVector2D>{}, VertexColors, TArray<FProcMeshTangent>{}, true);
	Chunk.Mesh->ContainsPhysicsTriMeshData(true);
	*/
}

void ATerrainManager::BeginPlay()
{
	Super::BeginPlay();

	//ChunkSize.X = 1;
	//ChunkSize.Y = 1;

	const auto Spawned = NewObject<UProceduralMeshComponent>(
				this,
				UProceduralMeshComponent::StaticClass(),
				UKismetStringLibrary::Conv_StringToName(FString::FromInt(0))
			);

	Spawned->RegisterComponent();
	Spawned->AttachToComponent(GetRootComponent(), FAttachmentTransformRules{EAttachmentRule::KeepWorld, false});
	Spawned->SetWorldLocation(FVector(0));
	Spawned->bUseAsyncCooking = true;
	Spawned->SetMaterial(0, TerrainMaterial);
	//CreateTriangle(Spawned);

	Chunks.Add(FVector2DInt{0,0}, FChunk{Spawned});
	/*
	ChunkSize.X = (ChunkRenderDistance * 2 + 1) * BlockSize;
	ChunkSize.Y = ChunkSize.X;
	ChunkBlockNum = ChunkSize.X * ChunkSize.Y;
	for (int X = -ChunkRenderDistance; X <= ChunkRenderDistance; ++X)
	{
		for (int Y = -ChunkRenderDistance; Y <= ChunkRenderDistance; ++Y)
		{
			const auto Spawned = NewObject<UProceduralMeshComponent>(
				this,
				UProceduralMeshComponent::StaticClass(),
				UKismetStringLibrary::Conv_StringToName(FString::FromInt(X))
			);

			Spawned->RegisterComponent();
			Spawned->AttachToComponent(GetRootComponent(), FAttachmentTransformRules{EAttachmentRule::KeepWorld, false});
			Spawned->SetWorldLocation(FVector(0));
			Spawned->SetWorldRotation(FRotator(-90,0,0));
			Spawned->bUseAsyncCooking = true;
			Spawned->SetMaterial(0, TerrainMaterial);
			CreateTriangle(Spawned);

			Chunks.Add(FVector2DInt{X,Y}, FChunk{Spawned});
		}
	}
	*/
}

void ATerrainManager::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const auto World = GetWorld();
	
	if (!World)
		return;

	RebuildChunk(Chunks[{0,0}], FVector(0));

	/*
	// Player Controller and pawn aren't cached to allow simple destruction. Can be changed for performance
	const auto PC = UGameplayStatics::GetPlayerController(World, 0);
	if (!PC)
		return;
	
	const auto Pawn = UGameplayStatics::GetPlayerPawn(World, 0);
	if (!Pawn)
		return;

	PlayerPosition = Pawn->GetActorLocation();

	// Auto Clamped to int
	PlayerIndex = {FMath::RoundToInt(PlayerPosition.X / ChunkSize.X), FMath::RoundToInt(PlayerPosition.Y / ChunkSize.Y)};

	
	// Update Chunks
	for (int32 X = -ChunkRenderDistance; X <= ChunkRenderDistance; ++X)
	{
		for (int32 Y = -ChunkRenderDistance; Y <= ChunkRenderDistance; ++Y)
		{
			const auto ChunkPosition = Chunks[{X, Y}].Mesh->GetRelativeLocation();
			//if (((ChunkPosition - PlayerPosition).GetAbs() - FVector(ChunkBlockSize.X / 2, ChunkBlockSize.Y / 2 ,0)).Size() > ChunkRenderDistance)
			if ((FVector2DInt{X, Y} - PlayerIndex).Abs().Size() > ChunkRenderDistance)
			//if ((ChunkPosition - PlayerPosition).GetAbs().Size() > ChunkRenderDistance)
			//if (PlayerIndex.Distance({X,Y}) > ChunkRenderDistance)
			{
				const auto NewPosition = FVector(
						(PlayerIndex.X + X) * ChunkSize.X,
						(PlayerIndex.Y + Y) * ChunkSize.Y ,
						0
					);
				Chunks[{X,Y}].Mesh->SetWorldLocation(NewPosition);
			}
			
			// Render And Collision
		}
	}*/
}


