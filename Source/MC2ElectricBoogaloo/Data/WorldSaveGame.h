// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "WorldSaveGame.generated.h"

DECLARE_DYNAMIC_DELEGATE(FWorldSaveDelegate);

UCLASS()
class MC2ELECTRICBOOGALOO_API UWorldSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure)
	static void GetSavedWorlds(TArray<FString>& Names);
	
	UFUNCTION(BlueprintCallable)
	static void SaveWorld(const FString& SaveName, const float& NewSeed, FWorldSaveDelegate OnSaved);
	UFUNCTION(BlueprintCallable)
	static void DeleteWorld(const FString& SaveName);
	UFUNCTION()
	static bool TryToLoadWorld(const FString& SaveName, float& NewSeed);
	
	UPROPERTY(SaveGame)
	float Seed;
};
