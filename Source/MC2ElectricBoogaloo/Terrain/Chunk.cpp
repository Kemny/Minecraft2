// Fill out your copyright notice in the Description page of Project Settings.

#include "Chunk.h"
#include "TerrainManager.h"
#include "ProceduralMeshComponent.h"

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
	static void RebuildGeometry(FThreadSafeBool& bCancelThread, const AChunk* const Chunk, const ATerrainManager* const Parent, const TMap<FVectorByte, FBlock>& Blocks, const FVectorByte& BlockCount, const FVector2DInt& WorldIndex, FChunkUpdateDelegateInternal Callback)
	{
		AsyncTask(ENamedThreads::AnyThread, [bCancelThread, Chunk, Parent, Blocks, BlockCount, WorldIndex, Callback]()
		{
			TArray<FVector2DInt> MissingDirections;
			FMeshInfo MeshInfo;
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
						AddPlane(MeshInfo, EBlockDirection::Up, Parent->GetTypeColor(Type), Index, Chunk);
				}
				// Down
				{
					if (Index.Z != 0)
					{
						AdjacentIndex = Index + FVectorByte(0,0,-1);
						if (Blocks.Contains(AdjacentIndex) && UBlockData::IsBlockTransparent(Blocks[AdjacentIndex].Type))
							AddPlane(MeshInfo, EBlockDirection::Down, Parent->GetTypeColor(Type), Index, Chunk);
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
						AddPlane(MeshInfo, EBlockDirection::Right, Parent->GetTypeColor(Type), Index, Chunk);
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
						AddPlane(MeshInfo, EBlockDirection::Left, Parent->GetTypeColor(Type), Index, Chunk);
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
						AddPlane(MeshInfo, EBlockDirection::Front, Parent->GetTypeColor(Type), Index, Chunk);
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
						AddPlane(MeshInfo, EBlockDirection::Back, Parent->GetTypeColor(Type), Index, Chunk);
				}
			}
			// ReSharper disable once CppExpressionWithoutSideEffects
			Callback.ExecuteIfBound(MeshInfo, MissingDirections);
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

void AChunk::InitializeVariables(ATerrainManager* NewParent)
{
	Parent = NewParent;
	const auto& BlockCount = Parent->GetBlockCount();
	
	MissingChunkDirections.Reserve(4);
	Blocks.Reserve(BlockCount.X * BlockCount.Y * BlockCount.Z);
}

void AChunk::RebuildBlocks(const FVector2DInt& Index)
{
	WorldIndex = Index;
	
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

				// TODO Seed Randomness
				Noise += Amplitude * FMath::PerlinNoise2D(Scale * ScaledPos);
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

void AChunk::OnBuildThreadFinished(const FMeshInfo& NewMeshInfo, const TArray<FVector2DInt>& MissingDirections)
{
	AsyncTask(ENamedThreads::GameThread, [NewMeshInfo, MissingDirections, this]()
	{
		if (bCancelThread)
		{
			bCancelThread = false;
			RebuildGeometry();
			return;
		}

		bIsThreadRunning = false;

		MissingChunkDirections = MissingDirections;
		MeshInfo = NewMeshInfo;
		
		//TODO One section per direction, don't reuse between sections to keep proper normals
		//TODO Create One, Then Update
		//TODO Array of Builders per direction
		//TODO Greedy Meshing
			Mesh->CreateMeshSection_LinearColor(
			0,
			MeshInfo.Vertices,
			MeshInfo.Triangles,
			MeshInfo.Normals,
			TArray<FVector2D>(),
			MeshInfo.VertexColors,
			TArray<FProcMeshTangent>(),
			true
		);

		OnUpdated.Broadcast(WorldIndex);
	});
}

void AChunk::Rebuild(const FVector2DInt& Index)
{
	bIsDirty = false;
	
	RebuildBlocks(Index);
	RebuildGeometry();
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

			// TODO Save System
			bIsDirty = true;

			TArray<FVector2DInt> Directions;
			if (IsBlockEdge(BlockIndex, Directions))
			{
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
