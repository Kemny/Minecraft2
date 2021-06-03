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

void AChunk::InitializeVariables(ATerrainManager* NewParent, const FVector2DInt& NewIndex)
{
	Parent = NewParent;
	Index = NewIndex;

	Rebuild({0,0});
}

void AChunk::AddPlane(FMeshBuilder& Builder, const FVector& Normal, const EBlockType& BlockType, const FVector& V1, const FVector& V2, const FVector& V3, const FVector& V4) const
{
	if (!Parent)
		return;

	const auto& Color = Parent->GetTypeColor(BlockType);
	
	const auto V1I = Builder.Vertices.AddUnique(V1);
	// If is new
	if (V1I == Builder.Vertices.Num() - 1)
	{
		Builder.Normals.Add(Normal);
		Builder.VertexColors.Add(Color);
	}
			
	const auto V2I = Builder.Vertices.AddUnique(V2);
	// If is new
	if (V2I == Builder.Vertices.Num() - 1)
	{
		Builder.Normals.Add(Normal);
		Builder.VertexColors.Add(Color);
	}
			
	const auto V3I = Builder.Vertices.AddUnique(V3);
	// If is new
	if (V3I == Builder.Vertices.Num() - 1)
	{
		Builder.Normals.Add(Normal);
		Builder.VertexColors.Add(Color);
	}
			
	const auto V4I = Builder.Vertices.AddUnique(V4);
	// If is new
	if (V4I == Builder.Vertices.Num() - 1)
	{
		Builder.Normals.Add(Normal);
		Builder.VertexColors.Add(Color);
	}
	
	Builder.Triangles.Add(V1I);
	Builder.Triangles.Add(V2I);
	Builder.Triangles.Add(V3I);
	Builder.Triangles.Add(V1I);
	Builder.Triangles.Add(V3I);
	Builder.Triangles.Add(V4I);
}

void AChunk::Rebuild(const FVector2DInt& PlayerIndex)
{
	if (!Parent)
		return;
	
	TArray<FBlockSpawnInfo> Heights;
	Parent->GetBlockHeights(Heights);
	
	if (Heights.Num() == 0)
		return;

	const auto& BlockSize = Parent->GetBlockSize();
	const auto& BlockCount = Parent->GetBlockCount();

	SetActorLocation(FVector(
		(Index.X + PlayerIndex.X) * BlockSize * BlockCount.X,
		(Index.Y + PlayerIndex.Y) * BlockSize * BlockCount.Y,
		0)
	);
	
	Blocks.Empty();
	
	for (uint8 X = 0; X < BlockCount.X; ++X)
	{
		for (uint8 Y = 0; Y < BlockCount.Y; ++Y)
		{
			for (uint8 Z = 0; Z < BlockCount.Z; ++Z)
			{
				float Noise = 0;
				const float Frequencies[]{ 0.1, 0.2, 0.4, 0.8, 0.16 };
				const float Amplitudes[]{ 1, 0.5f, 0.25f, 0.125f, 0.0625f };
				const auto Position = GetActorLocation();
				for (uint8 i = 0; i <= 5; ++i)
				{
					const auto NoiseResult = FMath::PerlinNoise2D(FVector2D(
						(Frequencies[i] * Position.X + .3f),
						(Frequencies[i] * Position.Y + .3f)
					));
					
					const auto NoiseNormalized = (NoiseResult + 1) / 2; // TODO Frequency and amplitude for correct normalization
					const auto NoiseAmplified = NoiseNormalized * Amplitudes[i];
					
					Noise += NoiseAmplified;
				}
				Noise -=.5f;
				const auto HeightNoise = Noise * BlockCount.Z;
				
				//Blocks.Add(FVectorByte(X,Y,Z), { Z == 0 ? EBlockType::Stone : EBlockType::Air});
				//Blocks.Add(FVectorByte(X,Y,Z), { Z > HeightNoise ? EBlockType::Air : EBlockType::Stone});

				UE_LOG(LogTemp, Warning, TEXT("%f"), Noise);
				if (Noise >= .7f)
					Blocks.Add(FVectorByte(X,Y,Z), { EBlockType::Air});
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

	// I sadly didn't find out how to properly do greedy meshing. At least UE4 lets you reuse vertices
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
				GetBlockLocalPosition(FVector(Block.Key.X + 1, Block.Key.Y, Block.Key.Z + 1)),
				GetBlockLocalPosition(FVector(Block.Key.X, Block.Key.Y, Block.Key.Z + 1)),
				GetBlockLocalPosition(FVector(Block.Key.X, Block.Key.Y + 1, Block.Key.Z + 1)),
				GetBlockLocalPosition(FVector(Block.Key.X + 1, Block.Key.Y + 1, Block.Key.Z + 1))
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
				GetBlockLocalPosition(FVector(Block.Key.X, Block.Key.Y + 1, Block.Key.Z)),
				GetBlockLocalPosition(FVector(Block.Key.X + 1, Block.Key.Y + 1, Block.Key.Z)),
				GetBlockLocalPosition(FVector(Block.Key.X + 1, Block.Key.Y + 1, Block.Key.Z + 1)),
				GetBlockLocalPosition(FVector(Block.Key.X, Block.Key.Y + 1, Block.Key.Z + 1))
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
				GetBlockLocalPosition(FVector(Block.Key.X + 1, Block.Key.Y + 1, Block.Key.Z)),
				GetBlockLocalPosition(FVector(Block.Key.X + 1, Block.Key.Y, Block.Key.Z)),
				GetBlockLocalPosition(FVector(Block.Key.X + 1, Block.Key.Y, Block.Key.Z + 1)),
				GetBlockLocalPosition(FVector(Block.Key.X + 1, Block.Key.Y + 1, Block.Key.Z + 1))
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
				GetBlockLocalPosition(FVector(Block.Key.X, Block.Key.Y, Block.Key.Z)),
				GetBlockLocalPosition(FVector(Block.Key.X + 1, Block.Key.Y, Block.Key.Z)),
				GetBlockLocalPosition(FVector(Block.Key.X + 1, Block.Key.Y + 1, Block.Key.Z)),
				GetBlockLocalPosition(FVector(Block.Key.X, Block.Key.Y + 1, Block.Key.Z))
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
				GetBlockLocalPosition(FVector(Block.Key.X + 1, Block.Key.Y, Block.Key.Z)),
				GetBlockLocalPosition(FVector(Block.Key.X, Block.Key.Y, Block.Key.Z)),
				GetBlockLocalPosition(FVector(Block.Key.X, Block.Key.Y, Block.Key.Z + 1)),
				GetBlockLocalPosition(FVector(Block.Key.X + 1, Block.Key.Y, Block.Key.Z + 1))
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
				GetBlockLocalPosition(FVector(Block.Key.X, Block.Key.Y, Block.Key.Z)),
				GetBlockLocalPosition(FVector(Block.Key.X, Block.Key.Y + 1, Block.Key.Z)),
				GetBlockLocalPosition(FVector(Block.Key.X, Block.Key.Y + 1, Block.Key.Z + 1)),
				GetBlockLocalPosition(FVector(Block.Key.X, Block.Key.Y, Block.Key.Z + 1))
			);
		}
	}
	Mesh->CreateMeshSection_LinearColor(0, Builder.Vertices, Builder.Triangles, Builder.Normals, TArray<FVector2D>{}, Builder.VertexColors, TArray<FProcMeshTangent>{}, true);
}

FVector AChunk::GetBlockLocalPosition(const FVector& BlockIndex) const
{
	if (!Parent)
		return {};

	const auto& BlockSize = Parent->GetBlockSize();
	return FVector(BlockIndex.X * BlockSize, BlockIndex.Y * BlockSize, BlockIndex.Z * BlockSize);
}
