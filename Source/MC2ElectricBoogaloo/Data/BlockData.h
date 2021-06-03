// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Structures.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "BlockData.generated.h"

UENUM(BlueprintType)
enum class EBlockType : uint8
{
	Air = 0,
	End = 1,
	Grass = 2,
	Dirt = 3,
	Stone = 4,
	Snow = 5
};

UENUM(BlueprintType)
enum class EBlockDirection : uint8
{
	Up = 0,
	Down = 1,
	Left = 2,
	Right = 3,
	Front = 4,
	Back = 5
};

USTRUCT(BlueprintType)
struct FBlockSpawnInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere)
	EBlockType Type;
	UPROPERTY(EditAnywhere)
	uint8 SpawnStopHeight;
};

USTRUCT(BlueprintType)
struct FBlock
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	EBlockType Type;
};

UCLASS()
class MC2ELECTRICBOOGALOO_API UBlockData : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION()
	static FVectorByte BlockDirectionVector(const EBlockDirection& Direction)
	{
		switch (Direction)
		{
		case EBlockDirection::Up: return FVectorByte(0,0,1);
		case EBlockDirection::Down: return FVectorByte(0,0,-1);
		case EBlockDirection::Left: return FVectorByte(0,-1,0);
		case EBlockDirection::Right: return FVectorByte(0,1,0);
		case EBlockDirection::Front: return FVectorByte(1,0,0);
		case EBlockDirection::Back: return FVectorByte(-1,0,0);
		default: return FVectorByte(0,0,0);
		}
	}
	UFUNCTION()
	static bool IsBlockTransparent(const EBlockType& Type)
	{
		switch (Type)
		{
		case EBlockType::Air: return true;
		case EBlockType::Grass: return false;
		case EBlockType::Dirt: return false;
		case EBlockType::Stone: return false;
		case EBlockType::Snow: return false;
		case EBlockType::End: return false;
		default: return true;
		}
	}
};
