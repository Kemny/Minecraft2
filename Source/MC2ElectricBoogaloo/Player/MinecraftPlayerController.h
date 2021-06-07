// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"

#include "MC2ElectricBoogaloo/Data/BlockData.h"

#include "MinecraftPlayerController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPlayerActionDelegate);

UENUM(BlueprintType)
enum class EPlayerState : uint8
{
	None = 0,
	Pickaxe = 1,
	PlaceBlock = 2
};

USTRUCT()
struct FSelectedPosition
{
	GENERATED_BODY()

	UPROPERTY()
	FVector Position;
	
	UPROPERTY()
	FVector Normal;
};

UCLASS()
class MC2ELECTRICBOOGALOO_API AMinecraftPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMinecraftPlayerController();
	
	UFUNCTION()
	FSelectedPosition GetSelectedPosition() const { return SelectedPosition; }

	UPROPERTY(BlueprintCallable)
	FPlayerActionDelegate OnPlaceBlockRequest;
	UPROPERTY(BlueprintCallable)
	FPlayerActionDelegate OnStartMining;
	UPROPERTY(BlueprintCallable)
	FPlayerActionDelegate OnStopMining;

	UFUNCTION()
	EPlayerState GetCurrentState() const { return CurrentState; }

	UFUNCTION()
	EBlockType GetSelectedBlock() const { return SelectedBlock; }

	UFUNCTION(BlueprintPure)
	bool IsMining() const { return bIsMining; }

	UFUNCTION()
	FVectorByte GetCurrentMiningBlock() const { return CurrentMiningBlock; }

	UFUNCTION()
	void SetCurrentMiningBlock(const FVectorByte& BlockIndex);

	UFUNCTION(BlueprintCallable)
	void UpdateMiningTime(const float& DeltaTime);
	UFUNCTION(BlueprintCallable)
	void ResetMiningTime();
	UFUNCTION(BlueprintCallable)
	float GetMiningTime() const { return MiningTime; }
	
protected:
	UPROPERTY()
	FVectorByte CurrentMiningBlock;
	
	UPROPERTY()
	bool bIsMining;

	UPROPERTY()
	float MiningTime;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float TraceRange = 1000;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	EPlayerState CurrentState;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	EBlockType SelectedBlock = EBlockType::Dirt;

	UPROPERTY()
	FSelectedPosition SelectedPosition;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION()
	void OnStartMiningFunction();
	
	UFUNCTION()
	void OnStopMiningFunction();
};
