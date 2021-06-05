// Fill out your copyright notice in the Description page of Project Settings.

#include "Chunk.h"
#include "TerrainManager.h"
#include "ProceduralMeshComponent.h"

const FVectorByte BackRightBottom(0,1,0);
const FVectorByte BackLeftBottom(0,0,0);
const FVectorByte FrontRightBottom(1,1,0);
const FVectorByte FrontLeftBottom(1,0,0);

const FVectorByte BackRightTop(0,1,1);
const FVectorByte BackLeftTop(0,0,1);
const FVectorByte FrontRightTop(1,1,1);
const FVectorByte FrontLeftTop(1,0,1);

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
	if (!Parent)
		return;
	
	const auto& BlockCount = Parent->GetBlockCount();
	Blocks.Reserve(BlockCount.X * BlockCount.Y * BlockCount.Z);
}

void AChunk::AddPlane(const EBlockDirection& Direction, const EBlockType& BlockType, const FVectorByte BlockIndex)
{
	if (!Parent)
		return;

	FVector Normal;
	FVector V1;
	FVector V2;
	FVector V3;
	FVector V4;
	
	switch (Direction)
	{
	case EBlockDirection::Up:
		Normal = FVector::UpVector;
		V1 = GetBlockLocalPosition(BlockIndex + FrontLeftTop);
		V2 = GetBlockLocalPosition(BlockIndex + BackLeftTop);
		V3 = GetBlockLocalPosition(BlockIndex + BackRightTop);
		V4 = GetBlockLocalPosition(BlockIndex + FrontRightTop);
		break;
	case EBlockDirection::Down:
		Normal = FVector::DownVector;
		V1 = GetBlockLocalPosition(BlockIndex + BackLeftBottom);
		V2 = GetBlockLocalPosition(BlockIndex + FrontLeftBottom);
		V3 = GetBlockLocalPosition(BlockIndex + FrontRightBottom);
		V4 = GetBlockLocalPosition(BlockIndex + BackRightBottom);
		break;
	case EBlockDirection::Left:
		Normal = FVector::LeftVector;
		V1 = GetBlockLocalPosition(BlockIndex + FrontLeftBottom);
		V2 = GetBlockLocalPosition(BlockIndex + BackLeftBottom);
		V3 = GetBlockLocalPosition(BlockIndex + BackLeftTop);
		V4 = GetBlockLocalPosition(BlockIndex + FrontLeftTop);
		break;
	case EBlockDirection::Right:
		Normal = FVector::RightVector;
		V1 = GetBlockLocalPosition(BlockIndex + BackRightBottom);
		V2 = GetBlockLocalPosition(BlockIndex + FrontRightBottom);
		V3 = GetBlockLocalPosition(BlockIndex + FrontRightTop);
		V4 = GetBlockLocalPosition(BlockIndex + BackRightTop);
		break;
	case EBlockDirection::Front:
		Normal = FVector::ForwardVector;
		V1 = GetBlockLocalPosition(BlockIndex + FrontRightBottom);
		V2 = GetBlockLocalPosition(BlockIndex + FrontLeftBottom);
		V3 = GetBlockLocalPosition(BlockIndex + FrontLeftTop);
		V4 = GetBlockLocalPosition(BlockIndex + FrontRightTop);
		break;
	case EBlockDirection::Back:
		Normal = FVector::BackwardVector;
		V1 = GetBlockLocalPosition(BlockIndex + BackLeftBottom);
		V2 = GetBlockLocalPosition(BlockIndex + BackRightBottom);
		V3 = GetBlockLocalPosition(BlockIndex + BackRightTop);
		V4 = GetBlockLocalPosition(BlockIndex + BackLeftTop);
		break;
	}

	const auto& Color = Parent->GetTypeColor(BlockType);

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

void AChunk::RebuildBlocks(const FVector2DInt& Index)
{
	WorldIndex = Index;
	
	if (!Parent)
		return;
	
	const auto& BlockSize = Parent->GetBlockSize();
	const auto& BlockCount = Parent->GetBlockCount();
	const auto& NoiseDamper = Parent->GetNoiseDamper();
	const auto& NoisePersistence = Parent->GetNoisePersistence();
	const auto& NoiseOctaves = Parent->GetNoiseOctaves();
	
	const auto NewPosition = FVector(
		WorldIndex.X * BlockSize * BlockCount.X,
		WorldIndex.Y * BlockSize * BlockCount.Y,
		0);
	
	SetActorLocation(NewPosition);

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
						if (BlockHeight.SpawnStopHeight > HeightNoise)
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
	MeshInfo.Clear();
	
	const auto& BlockCount = Parent->GetBlockCount();
	
	for (const auto & Block : Blocks)
	{
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
				AddPlane(EBlockDirection::Up, Type, Index);
		}
		// Down
		{
			if (Index.Z != 0)
			{
				AdjacentIndex = Index + FVectorByte(0,0,-1);
				if (Blocks.Contains(AdjacentIndex) && UBlockData::IsBlockTransparent(Blocks[AdjacentIndex].Type))
					AddPlane(EBlockDirection::Down, Type, Index);
			}
		}
		// Right
		{
			AdjacentIndex = Index + FVectorByte(0,1,0);
			if (Blocks.Contains(AdjacentIndex))
				bAdjacentTransparent = UBlockData::IsBlockTransparent(Blocks[AdjacentIndex].Type);
			else
			{
				bAdjacentTransparent = UBlockData::IsBlockTransparent(
					Parent->GetChunkBlockType(WorldIndex + FVector2DInt{0,1},
					FVectorByte(AdjacentIndex.X, 0, AdjacentIndex.Z)
					)
				);
			}
			if (bAdjacentTransparent)
				AddPlane(EBlockDirection::Right, Type, Index);
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
				bAdjacentTransparent = UBlockData::IsBlockTransparent(
					Parent->GetChunkBlockType(WorldIndex + FVector2DInt{0,-1},
					FVectorByte(AdjacentIndex.X, BlockCount.Y - 1, AdjacentIndex.Z)
					)
				);
			}
			
			if (bAdjacentTransparent)
				AddPlane(EBlockDirection::Left, Type, Index);
		}
		// Front
		{
			AdjacentIndex = Index + FVectorByte(1,0,0);
			if (Blocks.Contains(AdjacentIndex))
				bAdjacentTransparent = UBlockData::IsBlockTransparent(Blocks[AdjacentIndex].Type);
			else
			{
				bAdjacentTransparent = UBlockData::IsBlockTransparent(
					Parent->GetChunkBlockType(WorldIndex + FVector2DInt{1,0},
					FVectorByte(0, AdjacentIndex.Y, AdjacentIndex.Z)
					)
				);
			}
			if (bAdjacentTransparent)
				AddPlane(EBlockDirection::Front, Type, Index);
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
				bAdjacentTransparent = UBlockData::IsBlockTransparent(
					Parent->GetChunkBlockType(WorldIndex + FVector2DInt{-1,0},
					FVectorByte(BlockCount.X - 1, AdjacentIndex.Y, AdjacentIndex.Z)
					)
				);
			}
			
			if (bAdjacentTransparent)
				AddPlane(EBlockDirection::Back, Type, Index);
		}
	}
	//TODO One section per direction, don't reuse between sections to keep propper normals
	//TODO Create One, Then Update
	//TODO Array of Builders per direction 
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
}

void AChunk::Rebuild(const FVector2DInt& Index)
{
	if (!Parent)
		return;

	RebuildBlocks(Index);
	RebuildGeometry();
}

void AChunk::RemoveBlock(const FVectorByte& BlockIndex)
{
	if (!Parent)
		return;
	
	if (Blocks.Contains(BlockIndex))
	{
		const auto& Data = Parent->GetBlocksData();
		const auto& Type = Blocks[BlockIndex].Type;

		if (Data->Blocks.Contains(Type) && Data->Blocks[Type].bIsBreakable)
		{
			Blocks[BlockIndex].Type = EBlockType::Air;

			// TODO Don't rebuild everything
			RebuildGeometry();
		}
	}
}

void AChunk::AddBlock(const FVectorByte& BlockIndex, const EBlockType Type)
{
	if (!Parent)
		return;
	
	if (Blocks.Contains(BlockIndex))
	{
		const auto& OldType = Blocks[BlockIndex].Type;
		if (OldType == EBlockType::Air)
		{
			Blocks[BlockIndex].Type = Type;

			// TODO Don't rebuild everything
			RebuildGeometry();
		}
	}
}

FVector AChunk::GetBlockLocalPosition(const FVectorByte& BlockIndex) const
{
	if (!Parent)
		return {};

	const auto& BlockSize = Parent->GetBlockSize();
	return FVector(BlockIndex.X * BlockSize, BlockIndex.Y * BlockSize, BlockIndex.Z * BlockSize);
}

FVector AChunk::GetBlockWorldPosition(const FVectorByte& BlockIndex) const
{
	if (!Parent)
		return {};

	const auto& BlockSize = Parent->GetBlockSize();
	const auto& Location = GetActorLocation();
	return FVector(
		Location.X + BlockIndex.X * BlockSize, 
		Location.Y + BlockIndex.Y * BlockSize, 
		Location.Z + BlockIndex.Z * BlockSize
	);
}
