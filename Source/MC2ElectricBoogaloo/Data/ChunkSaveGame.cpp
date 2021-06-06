// Fill out your copyright notice in the Description page of Project Settings.

#include "ChunkSaveGame.h"
#include "Kismet/GameplayStatics.h"

// The save file size can be definitely lowered, but this is small enough and convenient for a demo
void UChunkSaveGame::SaveChunk(const FString& SaveName, const FVector2DInt& ChunkIndex, const TMap<FVectorByte, FBlock>& NewBlocks)
{
	if (UChunkSaveGame* SaveGameInstance = Cast<UChunkSaveGame>(UGameplayStatics::CreateSaveGameObject(StaticClass())))
	{
		SaveGameInstance->Blocks = NewBlocks;
		const auto& SavePath = FPaths::Combine(FPaths::ProjectSavedDir(), FString("Worlds"), SaveName, ChunkIndex.ToString() + ".sav");
		AsyncTask(ENamedThreads::AnyThread, [SaveGameInstance, SavePath]()
		{
			TArray<uint8> Characters;
			UGameplayStatics::SaveGameToMemory(SaveGameInstance, Characters);

			FFileHelper::SaveArrayToFile(Characters, *SavePath);
		});
	}
}

// Synchronous for ease of use. Can be changed if performance becomes and issue
bool UChunkSaveGame::TryToLoadChunk(const FString& SaveName, const FVector2DInt& ChunkIndex,TMap<FVectorByte, FBlock>& NewBlocks)
{
	const auto& FilePath = FPaths::Combine(FPaths::ProjectSavedDir(), FString("Worlds"), SaveName, ChunkIndex.ToString() + ".sav");
	
	if (!FPaths::FileExists(FilePath))
		return false;
	
	TArray<uint8> Characters;
	
	if (!FFileHelper::LoadFileToArray(Characters, *FilePath))
		return false;

	const auto& SaveGame = Cast<UChunkSaveGame>(UGameplayStatics::LoadGameFromMemory(Characters));
	if (!SaveGame)
		return false;

	NewBlocks = SaveGame->Blocks;
	return true;
}
