// Fill out your copyright notice in the Description page of Project Settings.

#include "Chunk.h"
#include "TerrainManager.h"
#include "ProceduralMeshComponent.h"

AChunk::AChunk()
{
	PrimaryActorTick.bCanEverTick = false;

	Mesh = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Mesh"));
	
	if (Mesh)
		SetRootComponent(Mesh);
}

void AChunk::InitializeVariables(ATerrainManager* NewParent, const FVector2DInt& Index)
{
	Parent = NewParent;
	if (!Parent)
		return;;
	
	const auto& BlockCount = Parent->GetBlockCount();
	Blocks.Reserve(BlockCount.X * BlockCount.Y * BlockCount.Z);
	
	Rebuild(Index);
}

void AChunk::AddPlane(FMeshBuilder& Builder, const FVector& Normal, const EBlockType& BlockType, const FVector& V1, const FVector& V2, const FVector& V3, const FVector& V4) const
{
	if (!Parent)
		return;

	const auto& Color = Parent->GetTypeColor(BlockType);

	const auto T1 = Builder.Triangles.Add(0);
	const auto T2 = Builder.Triangles.Add(1);
	const auto T3 = Builder.Triangles.Add(2);
	const auto T4 = Builder.Triangles.Add(0);
	const auto T5 = Builder.Triangles.Add(2);
	const auto T6 = Builder.Triangles.Add(3);
	
	int32 Index;
	
	if (!Builder.Vertices.Find(V1, Index))
	{
		Index = Builder.Vertices.Add(V1);
		Builder.Normals.Add(Normal);
		Builder.VertexColors.Add(Color);
	}
	Builder.Triangles[T1] = Index;
	Builder.Triangles[T4] = Index;
	
	if (!Builder.Vertices.Find(V2, Index))
	{
		Index = Builder.Vertices.Add(V2);
		Builder.Normals.Add(Normal);
		Builder.VertexColors.Add(Color);
	}
	Builder.Triangles[T2] = Index;
	
	if (!Builder.Vertices.Find(V3, Index))
	{
		Index = Builder.Vertices.Add(V3);
		Builder.Normals.Add(Normal);
		Builder.VertexColors.Add(Color);
	}
	Builder.Triangles[T3] = Index;
	Builder.Triangles[T5] = Index;
	
	if (!Builder.Vertices.Find(V4, Index))
	{
		Index = Builder.Vertices.Add(V4);
		Builder.Normals.Add(Normal);
		Builder.VertexColors.Add(Color);
	}
	Builder.Triangles[T6] = Index;
}

void AChunk::Rebuild(const FVector2DInt& Index)
{
	WorldIndex = Index;
	
	if (!Parent)
		return;
	
	TArray<FBlockSpawnInfo> Heights;
	Parent->GetBlockHeights(Heights);
	
	if (Heights.Num() == 0)
		return;

	const auto& BlockSize = Parent->GetBlockSize();
	const auto& BlockCount = Parent->GetBlockCount();
	const auto& NoiseDamper = Parent->GetNoiseDamper();
	const auto& NoisePersistence = Parent->GetNoisePersistence();
	const auto& NoiseOctaves = Parent->GetNoiseOctaves();

	/*
	if (NoiseDamper <= 0 || NoiseOctaves <= 0)
		return;
	*/
	
	const auto NewPosition = FVector(
		WorldIndex.X * BlockSize * BlockCount.X,
		WorldIndex.Y * BlockSize * BlockCount.Y,
		0);
	
	SetActorLocation(NewPosition);

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
				//if (Z <= HeightNoise) Cool looking effect
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

	FMeshBuilder Builder;
	for (const auto & Block : Blocks)
	{
		if (Block.Value.Type == EBlockType::Air)
			continue;

		// Up
		const auto UpIndex = Block.Key + FVectorByte{0,0,1};
		if (!Blocks.Contains(UpIndex) || Blocks.Contains(UpIndex) && UBlockData::IsBlockTransparent(Blocks[UpIndex].Type))
		{
			AddPlane(
				Builder,
				FVector::UpVector,
				Block.Value.Type,
				GetBlockLocalPosition(FVectorByte(Block.Key.X + 1, Block.Key.Y, Block.Key.Z + 1)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X, Block.Key.Y, Block.Key.Z + 1)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X, Block.Key.Y + 1, Block.Key.Z + 1)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X + 1, Block.Key.Y + 1, Block.Key.Z + 1))
			);
		}
		
		// Right
		const auto RightIndex = Block.Key + FVectorByte{0,1,0};
		if (Blocks.Contains(RightIndex) && UBlockData::IsBlockTransparent(Blocks[RightIndex].Type))
		{
			AddPlane(
				Builder,
				FVector::RightVector,
				Block.Value.Type,
				GetBlockLocalPosition(FVectorByte(Block.Key.X, Block.Key.Y + 1, Block.Key.Z)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X + 1, Block.Key.Y + 1, Block.Key.Z)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X + 1, Block.Key.Y + 1, Block.Key.Z + 1)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X, Block.Key.Y + 1, Block.Key.Z + 1))
			);
		}
		// Forward
		const auto ForwardIndex = Block.Key + FVectorByte{1,0,0};
		if (Blocks.Contains(ForwardIndex) && UBlockData::IsBlockTransparent(Blocks[ForwardIndex].Type))
		{
			AddPlane(
				Builder,
				FVector::ForwardVector,
				Block.Value.Type,
				GetBlockLocalPosition(FVectorByte(Block.Key.X + 1, Block.Key.Y + 1, Block.Key.Z)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X + 1, Block.Key.Y, Block.Key.Z)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X + 1, Block.Key.Y, Block.Key.Z + 1)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X + 1, Block.Key.Y + 1, Block.Key.Z + 1))
			);
		}
		
		// Down
		if (Block.Key.Z != 0 && Blocks.Contains(FVectorByte(Block.Key.X, Block.Key.Y, Block.Key.Z - 1)) &&
			UBlockData::IsBlockTransparent(Blocks[FVectorByte(Block.Key.X, Block.Key.Y, Block.Key.Z - 1)].Type))
		{
			AddPlane(
				Builder,
				FVector::DownVector,
				Block.Value.Type,
				GetBlockLocalPosition(FVectorByte(Block.Key.X, Block.Key.Y, Block.Key.Z)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X + 1, Block.Key.Y, Block.Key.Z)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X + 1, Block.Key.Y + 1, Block.Key.Z)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X, Block.Key.Y + 1, Block.Key.Z))
			);
		}
		// Left
		if (Block.Key.Y != 0 && Blocks.Contains(FVectorByte(Block.Key.X, Block.Key.Y - 1, Block.Key.Z)) &&
			UBlockData::IsBlockTransparent(Blocks[FVectorByte(Block.Key.X, Block.Key.Y - 1, Block.Key.Z)].Type))
		{
			AddPlane(
				Builder,
				FVector::LeftVector,
				Block.Value.Type,
				GetBlockLocalPosition(FVectorByte(Block.Key.X + 1, Block.Key.Y, Block.Key.Z)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X, Block.Key.Y, Block.Key.Z)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X, Block.Key.Y, Block.Key.Z + 1)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X + 1, Block.Key.Y, Block.Key.Z + 1))
			);
		}
		
		// Backward
		if (Block.Key.X != 0 && Blocks.Contains(FVectorByte(Block.Key.X - 1, Block.Key.Y, Block.Key.Z)) &&
			UBlockData::IsBlockTransparent(Blocks[FVectorByte(Block.Key.X - 1, Block.Key.Y, Block.Key.Z)].Type))
		{
			AddPlane(
				Builder,
				FVector::BackwardVector,
				Block.Value.Type,
				GetBlockLocalPosition(FVectorByte(Block.Key.X, Block.Key.Y, Block.Key.Z)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X, Block.Key.Y + 1, Block.Key.Z)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X, Block.Key.Y + 1, Block.Key.Z + 1)),
				GetBlockLocalPosition(FVectorByte(Block.Key.X, Block.Key.Y, Block.Key.Z + 1))
			);
		}
	}
	Mesh->CreateMeshSection_LinearColor(0, Builder.Vertices, Builder.Triangles, Builder.Normals, TArray<FVector2D>{}, Builder.VertexColors, TArray<FProcMeshTangent>{}, true);
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
