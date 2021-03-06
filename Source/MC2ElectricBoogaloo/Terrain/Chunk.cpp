// Fill out your copyright notice in the Description page of Project Settings.

#include "Chunk.h"
#include "TerrainManager.h"
#include "ProceduralMeshComponent.h"
#include "MC2ElectricBoogaloo/Data/ChunkSaveGame.h"

namespace BuildTask
{
	const FVectorByte BackRightBottom(0,1,0);
	const FVectorByte BackLeftBottom(0,0,0);
	const FVectorByte FrontRightBottom(1,1,0);
	const FVectorByte FrontLeftBottom(1,0,0);

	const FVectorByte BackRightTop(0,1,1);
	const FVectorByte BackLeftTop(0,0,1);
	const FVectorByte FrontRightTop(1,1,1);
	const FVectorByte FrontLeftTop(1,0,1);

	// Add 4 vertices and reuse triangles at same position
	static void AddPlane(FMeshInfo& MeshInfo, const EBlockDirection& Direction, const FLinearColor& Color, const FVectorByte& BlockIndex, const AChunk* const Chunk)
	{
		FVector Normal;
		FVector V1;
		FVector V2;
		FVector V3;
		FVector V4;
		
		switch (Direction)
		{
		case EBlockDirection::Up:
			Normal = FVector::UpVector;
			V1 = Chunk->GetBlockLocalPosition(BlockIndex + FrontLeftTop);
			V2 = Chunk->GetBlockLocalPosition(BlockIndex + BackLeftTop);
			V3 = Chunk->GetBlockLocalPosition(BlockIndex + BackRightTop);
			V4 = Chunk->GetBlockLocalPosition(BlockIndex + FrontRightTop);
			break;
		case EBlockDirection::Down:
			Normal = FVector::DownVector;
			V1 = Chunk->GetBlockLocalPosition(BlockIndex + BackLeftBottom);
			V2 = Chunk->GetBlockLocalPosition(BlockIndex + FrontLeftBottom);
			V3 = Chunk->GetBlockLocalPosition(BlockIndex + FrontRightBottom);
			V4 = Chunk->GetBlockLocalPosition(BlockIndex + BackRightBottom);
			break;
		case EBlockDirection::Left:
			Normal = FVector::LeftVector;
			V1 = Chunk->GetBlockLocalPosition(BlockIndex + FrontLeftBottom);
			V2 = Chunk->GetBlockLocalPosition(BlockIndex + BackLeftBottom);
			V3 = Chunk->GetBlockLocalPosition(BlockIndex + BackLeftTop);
			V4 = Chunk->GetBlockLocalPosition(BlockIndex + FrontLeftTop);
			break;
		case EBlockDirection::Right:
			Normal = FVector::RightVector;
			V1 = Chunk->GetBlockLocalPosition(BlockIndex + BackRightBottom);
			V2 = Chunk->GetBlockLocalPosition(BlockIndex + FrontRightBottom);
			V3 = Chunk->GetBlockLocalPosition(BlockIndex + FrontRightTop);
			V4 = Chunk->GetBlockLocalPosition(BlockIndex + BackRightTop);
			break;
		case EBlockDirection::Front:
			Normal = FVector::ForwardVector;
			V1 = Chunk->GetBlockLocalPosition(BlockIndex + FrontRightBottom);
			V2 = Chunk->GetBlockLocalPosition(BlockIndex + FrontLeftBottom);
			V3 = Chunk->GetBlockLocalPosition(BlockIndex + FrontLeftTop);
			V4 = Chunk->GetBlockLocalPosition(BlockIndex + FrontRightTop);
			break;
		case EBlockDirection::Back:
			Normal = FVector::BackwardVector;
			V1 = Chunk->GetBlockLocalPosition(BlockIndex + BackLeftBottom);
			V2 = Chunk->GetBlockLocalPosition(BlockIndex + BackRightBottom);
			V3 = Chunk->GetBlockLocalPosition(BlockIndex + BackRightTop);
			V4 = Chunk->GetBlockLocalPosition(BlockIndex + BackLeftTop);
			break;
		}

		const auto T1 = MeshInfo.Triangles.Add(0);
		const auto T2 = MeshInfo.Triangles.Add(1);
		const auto T3 = MeshInfo.Triangles.Add(2);
		const auto T4 = MeshInfo.Triangles.Add(0);
		const auto T5 = MeshInfo.Triangles.Add(2);
		const auto T6 = MeshInfo.Triangles.Add(3);
		
		int32 Index;
		
		if (!MeshInfo.Vertices.Find(V1, Index))
		{
			Index = MeshInfo.Vertices.Add(V1);
			MeshInfo.Normals.Add(Normal);
			MeshInfo.VertexColors.Add(Color);
		}
		MeshInfo.Triangles[T1] = Index;
		MeshInfo.Triangles[T4] = Index;
		
		if (!MeshInfo.Vertices.Find(V2, Index))
		{
			Index = MeshInfo.Vertices.Add(V2);
			MeshInfo.Normals.Add(Normal);
			MeshInfo.VertexColors.Add(Color);
		}
		MeshInfo.Triangles[T2] = Index;
		
		if (!MeshInfo.Vertices.Find(V3, Index))
		{
			Index = MeshInfo.Vertices.Add(V3);
			MeshInfo.Normals.Add(Normal);
			MeshInfo.VertexColors.Add(Color);
		}
		MeshInfo.Triangles[T3] = Index;
		MeshInfo.Triangles[T5] = Index;
		
		if (!MeshInfo.Vertices.Find(V4, Index))
		{
			Index = MeshInfo.Vertices.Add(V4);
			MeshInfo.Normals.Add(Normal);
			MeshInfo.VertexColors.Add(Color);
		}
		MeshInfo.Triangles[T6] = Index;
	}
	// Generate Mesh information on another thread
	static void RebuildGeometry(FThreadSafeBool& bCancelThread, const AChunk* const Chunk, const ATerrainManager* const Parent, const TMap<FVectorByte, FBlock>& Blocks, const FVectorByte& BlockCount, const FVector2DInt& WorldIndex, FChunkUpdateDelegateInternal Callback)
	{
		// I couldn't figure out the Greedy Meshing algorithm
		AsyncTask(ENamedThreads::AnyThread, [bCancelThread, Chunk, Parent, Blocks, BlockCount, WorldIndex, Callback]()
		{
			TArray<FVector2DInt> MissingDirections;

			// Not using the old one for easy thread safety
			TArray<FMeshInfo> MeshInfos;
			MeshInfos.Init({}, 3);
			
			for (const auto & Block : Blocks)
			{
				if (bCancelThread)
					break;
				
				if (Block.Value.Type == EBlockType::Air)
					continue;

				const auto& Index = Block.Key;
				const auto& Type = Block.Value.Type;

				FVectorByte AdjacentIndex;
				bool bAdjacentTransparent;
				// Up
				{
					AdjacentIndex = Index +  FVectorByte(0,0,1);
					if (Blocks.Contains(AdjacentIndex) && UBlockData::IsBlockTransparent(Blocks[AdjacentIndex].Type) || !Blocks.Contains(AdjacentIndex))
						AddPlane(MeshInfos[0], EBlockDirection::Up, Parent->GetTypeColor(Type), Index, Chunk);
				}
				// Down
				{
					if (Index.Z != 0)
					{
						AdjacentIndex = Index + FVectorByte(0,0,-1);
						if (Blocks.Contains(AdjacentIndex) && UBlockData::IsBlockTransparent(Blocks[AdjacentIndex].Type))
							AddPlane(MeshInfos[0], EBlockDirection::Down, Parent->GetTypeColor(Type), Index, Chunk);
					}
				}
				// Right
				{
					AdjacentIndex = Index + FVectorByte(0,1,0);
					if (Blocks.Contains(AdjacentIndex))
						bAdjacentTransparent = UBlockData::IsBlockTransparent(Blocks[AdjacentIndex].Type);
					else
					{
						FBlock FoundBlock;
						const FVector2DInt ChunkDirection{0,1};
						const FVectorByte BlockIndex(AdjacentIndex.X, 0, AdjacentIndex.Z);
						
						if (Parent->GetBlockSafe(WorldIndex + ChunkDirection, BlockIndex, FoundBlock))
							bAdjacentTransparent = UBlockData::IsBlockTransparent(FoundBlock.Type);
						else
						{
							bAdjacentTransparent = false;
							MissingDirections.AddUnique(ChunkDirection);
						}
						
					}
					if (bAdjacentTransparent)
						AddPlane(MeshInfos[1], EBlockDirection::Right, Parent->GetTypeColor(Type), Index, Chunk);
				}
				// Left
				{
					if (Index.Y != 0)
					{
						AdjacentIndex = Index + FVectorByte(0,-1,0);
						bAdjacentTransparent = UBlockData::IsBlockTransparent(Blocks[AdjacentIndex].Type);
					}
					else
					{
						FBlock FoundBlock;
						const FVector2DInt ChunkDirection{0,-1};
						const FVectorByte BlockIndex(AdjacentIndex.X, BlockCount.Y - 1, AdjacentIndex.Z);
						
						if (Parent->GetBlockSafe(WorldIndex + ChunkDirection, BlockIndex, FoundBlock))
							bAdjacentTransparent = UBlockData::IsBlockTransparent(FoundBlock.Type);
						else
						{
							bAdjacentTransparent = false;
							MissingDirections.AddUnique(ChunkDirection);
						}
					}
					
					if (bAdjacentTransparent)
						AddPlane(MeshInfos[1], EBlockDirection::Left, Parent->GetTypeColor(Type), Index, Chunk);
				}
				// Front
				{
					AdjacentIndex = Index + FVectorByte(1,0,0);
					if (Blocks.Contains(AdjacentIndex))
						bAdjacentTransparent = UBlockData::IsBlockTransparent(Blocks[AdjacentIndex].Type);
					else
					{
						FBlock FoundBlock;
						const FVector2DInt ChunkDirection{1,0};
						const FVectorByte BlockIndex(0, AdjacentIndex.Y, AdjacentIndex.Z);

						if (Parent->GetBlockSafe(WorldIndex + ChunkDirection, BlockIndex, FoundBlock))
							bAdjacentTransparent = UBlockData::IsBlockTransparent(FoundBlock.Type);
						else
						{
							bAdjacentTransparent = false;
							MissingDirections.AddUnique(ChunkDirection);
						}
					}
					if (bAdjacentTransparent)
						AddPlane(MeshInfos[2], EBlockDirection::Front, Parent->GetTypeColor(Type), Index, Chunk);
				}
				// Back
				{
					if (Index.X != 0)
					{
						AdjacentIndex = Index + FVectorByte(-1,0,0);
						bAdjacentTransparent = UBlockData::IsBlockTransparent(Blocks[AdjacentIndex].Type);
					}
					else
					{
						
						FBlock FoundBlock;
						const FVector2DInt ChunkDirection{-1,0};
						const FVectorByte BlockIndex(BlockCount.X - 1, AdjacentIndex.Y, AdjacentIndex.Z);

						if (Parent->GetBlockSafe(WorldIndex + ChunkDirection, BlockIndex, FoundBlock))
							bAdjacentTransparent = UBlockData::IsBlockTransparent(FoundBlock.Type);
						else
						{
							bAdjacentTransparent = false;
							MissingDirections.AddUnique(ChunkDirection);
						}
					}
					
					if (bAdjacentTransparent)
						AddPlane(MeshInfos[2], EBlockDirection::Back, Parent->GetTypeColor(Type), Index, Chunk);
				}
			}
			AsyncTask(ENamedThreads::GameThread, [Callback, MeshInfos, MissingDirections]()
			{
				// ReSharper disable once CppExpressionWithoutSideEffects
				Callback.ExecuteIfBound(MeshInfos, MissingDirections);
			});
		});
	}
}

AChunk::AChunk()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	
	if (Mesh)
		SetRootComponent(Mesh);
}

// Set Variables, and reserve space
void AChunk::InitializeVariables(ATerrainManager* NewParent)
{
	Parent = NewParent;
	const auto& BlockCount = Parent->GetBlockCount();

	for (uint8 i = 0; i < 3; ++i)
		Mesh->SetMaterial(i, Material);
	
	MissingChunkDirections.Reserve(4);
	Blocks.Reserve(BlockCount.X * BlockCount.Y * BlockCount.Z);
}

// If there is something to save, do so
void AChunk::TryToSave() const
{
	if (bIsDirty)
		UChunkSaveGame::SaveChunk(Parent->GetWorldName(), WorldIndex, Blocks);
}

// Generate Block information for a new chunk index
void AChunk::BuildBlocks(const FVector2DInt& Index)
{
	TryToSave();

	WorldIndex = Index;
	bIsDirty = false;
	
	const auto& BlockSize = Parent->GetBlockSize();
	const auto& BlockCount = Parent->GetBlockCount();
	const auto& NoiseDamper = Parent->GetNoiseDamper();
	const auto& NoisePersistence = Parent->GetNoisePersistence();
	const auto& NoiseOctaves = Parent->GetNoiseOctaves();

	TArray<FBlockSpawnInfo> Heights;
	Parent->GetBlockHeights(Heights);

	if (NoiseDamper <= 0 || NoiseOctaves <= 0 || Heights.Num() == 0)
		return;
	
	Blocks.Empty(BlockCount.X * BlockCount.Y * BlockCount.Z);

	if (UChunkSaveGame::TryToLoadChunk(Parent->GetWorldName(), WorldIndex, Blocks))
		return;
	
	for (uint8 X = 0; X < BlockCount.X; ++X)
	{
		for (uint8 Y = 0; Y < BlockCount.Y; ++Y)
		{
			Blocks.Add(FVectorByte(X,Y,0), {EBlockType::End});
		}
	}
	
	
	for (uint8 X = 0; X < BlockCount.X; ++X)
	{
		for (uint8 Y = 0; Y < BlockCount.Y; ++Y)
		{
			float Scale = 1;
			float Amplitude = 1;
			float TotalAmplitude = 0;
			
			const auto& Pos = GetBlockWorldPosition({X,Y,0});
			const FVector2D ScaledPos{Pos.X / NoiseDamper / BlockSize, Pos.Y / NoiseDamper / BlockSize };
			
			float Noise = 0;
			for (int32 i = NoiseOctaves - 1; i >= 0; --i)
			{
				Amplitude /= NoisePersistence;
				Scale *= NoisePersistence;
				TotalAmplitude += Amplitude;

				const auto NoisePos = Scale * ScaledPos;
				Noise += Amplitude * FMath::PerlinNoise3D(FVector(NoisePos.X, NoisePos.Y, Parent->GetWorldSeed() * Parent->GetSeedNoiseWeight()));
			}

			Noise /= TotalAmplitude;
			Noise = (Noise + 1) / 2;
			const auto HeightNoise = Noise * BlockCount.Z;
			for (uint8 Z = 1; Z < BlockCount.Z; ++Z)
			{
				if (Z > HeightNoise)
				{
					Blocks.Add(FVectorByte(X,Y,Z), { EBlockType::Air});
				}
				else
				{
					for (const auto & BlockHeight : Heights)
					{
						if (BlockHeight.SpawnStopHeight > Z)
						{
							Blocks.Add(FVectorByte(X,Y,Z), {BlockHeight.Type});
							break;
						}
					}
				}
			}
		}
	}
}

// Rebuild Meshes for all directions
void AChunk::RebuildGeometry()
{
	static FChunkUpdateDelegateInternal OnRebuiltInternal;

	if (bCancelThread)
	{
		bCancelThread = false;
		return;
	}
	bIsThreadRunning = true;
	
	OnRebuiltInternal.BindDynamic(this, &AChunk::OnBuildThreadFinished);
	const auto& BlockCount = Parent->GetBlockCount();
	BuildTask::RebuildGeometry(bCancelThread, this, Parent, Blocks, BlockCount, WorldIndex, OnRebuiltInternal);
}

// When the build thread is finished, pass the mesh info to unreal 
void AChunk::OnBuildThreadFinished(const TArray<FMeshInfo>& NewMeshInfos, const TArray<FVector2DInt>& MissingDirections)
{
	if (bCancelThread)
	{
		bCancelThread = false;
		RebuildGeometry();
		return;
	}

	bIsThreadRunning = false;

	MissingChunkDirections = MissingDirections;
	MeshInfos = NewMeshInfos;
	
	for (uint8 i = 0; i < MeshInfos.Num(); ++i)
	{
		const auto & MeshInfo = MeshInfos[i];
		// This function cannot be put on another thread nor is there a delegate for async collisions
		Mesh->CreateMeshSection_LinearColor(
			i,
			MeshInfo.Vertices,
			MeshInfo.Triangles,
			MeshInfo.Normals,
			TArray<FVector2D>(),
			MeshInfo.VertexColors,
			TArray<FProcMeshTangent>(),
			true
		);
	}

	OnUpdated.Broadcast(WorldIndex);
}

void AChunk::RemoveBlock(const FVectorByte& BlockIndex)
{
	if (Blocks.Contains(BlockIndex))
	{
		const auto& Data = Parent->GetBlocksData();
		const auto& Type = Blocks[BlockIndex].Type;

		if (Data->Blocks.Contains(Type) && Data->Blocks[Type].bIsBreakable)
		{
			Blocks[BlockIndex].Type = EBlockType::Air;

			RebuildGeometry();

			bIsDirty = true;

			TArray<FVector2DInt> Directions;
			if (IsBlockEdge(BlockIndex, Directions))
			{
				// ReSharper disable once CppExpressionWithoutSideEffects
				OnEdgeUpdated.ExecuteIfBound(WorldIndex, Directions);
			}
		}
	}
}
void AChunk::AddBlock(const FVectorByte& BlockIndex, const EBlockType Type)
{
	if (Blocks.Contains(BlockIndex))
	{
		const auto& OldType = Blocks[BlockIndex].Type;
		if (OldType == EBlockType::Air)
		{
			Blocks[BlockIndex].Type = Type;

			RebuildGeometry();
			
			bIsDirty = true;

			TArray<FVector2DInt> Directions;
			if (IsBlockEdge(BlockIndex, Directions))
			{
				// ReSharper disable once CppExpressionWithoutSideEffects
				OnEdgeUpdated.ExecuteIfBound(WorldIndex, Directions);
			}
		}
	}
}

bool AChunk::IsBlockEdge(const FVectorByte& BlockIndex, TArray<FVector2DInt>& EdgeDirections) const
{
	const auto& BlockCount = Parent->GetBlockCount();
	if (BlockIndex.X == 0)
		EdgeDirections.Add({-1, 0});
	
	if (BlockIndex.X == BlockCount.X - 1)
		EdgeDirections.Add({1, 0});
	
	if (BlockIndex.Y == 0)
		EdgeDirections.Add({0, -1});
	
	if (BlockIndex.Y == BlockCount.Y - 1)
		EdgeDirections.Add({0, 1});
	
	return EdgeDirections.Num() > 0;
}

FVector AChunk::GetBlockLocalPosition(const FVectorByte& BlockIndex) const
{
	const auto& BlockSize = Parent->GetBlockSize();
	return FVector(BlockIndex.X * BlockSize, BlockIndex.Y * BlockSize, BlockIndex.Z * BlockSize);
}
FVector AChunk::GetBlockWorldPosition(const FVectorByte& BlockIndex) const
{
	const auto& BlockSize = Parent->GetBlockSize();
	const auto& BlockCount = Parent->GetBlockCount();

	return FVector(
		WorldIndex.X * BlockSize * BlockCount.X + BlockIndex.X * BlockSize, 
		WorldIndex.Y * BlockSize * BlockCount.Y + BlockIndex.Y * BlockSize, 
		BlockIndex.Z * BlockSize
	);
}
