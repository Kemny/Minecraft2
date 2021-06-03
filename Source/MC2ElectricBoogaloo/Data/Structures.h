// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Structures.generated.h"

USTRUCT(BlueprintType)
struct FVector2DByte
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 X;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Y;

	FVector2DByte(): X(0), Y(0)
	{ }
	FVector2DByte(const uint8& X, const uint8& Y): X(X), Y(Y)
	{ }

	bool operator==(const FVector2DByte& Other) const
	{
		return X == Other.X && Y == Other.Y;
	}
	bool Equals(const FVector2DByte& Other) const
	{
		return X == Other.X && Y == Other.Y;
	}
};
USTRUCT(BlueprintType)
struct FVector2DInt
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 X;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Y;

	bool operator==(const FVector2DInt& Other) const
	{
		return X == Other.X && Y == Other.Y;
	}
	bool operator!=(const FVector2DInt& Other) const
	{
		return X != Other.X || Y != Other.Y;
	}
	bool Equals(const FVector2DInt& Other) const
	{
		return X == Other.X && Y == Other.Y;
	}
	float Distance(const FVector2DInt& Other) const
	{
		return FMath::Sqrt(FMath::Square(Other.X- X) + FMath::Square(Other.Y- Y));
	}
	FVector2DInt Abs() const
	{
		return FVector2DInt{FMath::Abs(X), FMath::Abs(Y) };
	}
	float Size() const
	{
		return FMath::Sqrt(X*X + Y*Y);
	}
	FVector2DInt operator+(const FVector2DInt& Other) const
	{
		return FVector2DInt{X + Other.X, Y + Other.Y};
	}
	FVector2DInt operator+(const FVector& Other) const
	{
		return FVector2DInt{X + static_cast<int32>(Other.X), Y + static_cast<int32>(Other.Y)};
	}
	FVector2DInt operator-(const FVector2DInt& Other) const
	{
		return FVector2DInt{X - Other.X, Y - Other.Y};
	}
	FVector2DInt operator-(const FVector& Other) const
	{
		return FVector2DInt{X - static_cast<int32>(Other.X), Y - static_cast<int32>(Other.Y)};
	}
};

USTRUCT(BlueprintType)
struct FVectorByte
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 X;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Y;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	uint8 Z;

	FVectorByte()
		: X(0), Y(0), Z(0)
	{ }

	FVectorByte(const uint8& X, const uint8& Y, const uint8& Z)
		: X(X), Y(Y), Z(Z)
	{ }

	bool operator==(const FVectorByte& Other) const
	{
		return X == Other.X && Y == Other.Y && Z == Other.Z;
	}
	bool Equals(const FVectorByte& Other) const
	{
		return X == Other.X && Y == Other.Y && Z == Other.Z;
	}
	FVectorByte operator+(const FVectorByte& Other) const
	{
		return FVectorByte(X + Other.X, Y + Other.Y, Z + Other.Z);
	}
	FVectorByte operator*(const FVectorByte& Other) const
	{
		return FVectorByte(X * Other.X, Y * Other.Y, Z * Other.Z);
	}
};

USTRUCT(BlueprintType)
struct FVectorInt
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 X;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Y;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Z;

	bool operator==(const FVectorInt& Other) const
	{
		return X == Other.X && Y == Other.Y && Z == Other.Z;
	}
	bool Equals(const FVectorInt& Other) const
	{
		return X == Other.X && Y == Other.Y && Z == Other.Z;
	}
};

FORCEINLINE uint32 GetTypeHash(const FVector2DInt& Thing) { return FCrc::MemCrc32(&Thing, sizeof(FVector2DInt)); }
FORCEINLINE uint32 GetTypeHash(const FVectorInt& Thing) { return FCrc::MemCrc32(&Thing, sizeof(FVectorInt)); }
FORCEINLINE uint32 GetTypeHash(const FVector2DByte& Thing) { return FCrc::MemCrc32(&Thing, sizeof(FVector2DByte)); }
FORCEINLINE uint32 GetTypeHash(const FVectorByte& Thing) { return FCrc::MemCrc32(&Thing, sizeof(FVectorByte)); }

UCLASS()
class MC2ELECTRICBOOGALOO_API UStructures : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
};
