// Fill out your copyright notice in the Description page of Project Settings.

#include "WorldSaveGame.h"
#include "Kismet/GameplayStatics.h"

struct DirectoryVisitor : IPlatformFile::FDirectoryVisitor
{
	TArray<FString> DirectoryNames;
	virtual bool Visit(const TCHAR* FilenameOrDirectory, bool bIsDirectory) override
	{
		if(bIsDirectory)
			DirectoryNames.Add(FPaths::GetCleanFilename(FilenameOrDirectory));
		
		return true;
	}
};

void UWorldSaveGame::GetSavedWorlds(TArray<FString>& Names)
{
	DirectoryVisitor Visitor;
	FPlatformFileManager::Get().GetPlatformFile().IterateDirectory(*FPaths::Combine(FPaths::ProjectSavedDir(), FString("Worlds")),Visitor);
	Names = Visitor.DirectoryNames;
}

void UWorldSaveGame::SaveWorld(const FString& SaveName, const float& NewSeed, FWorldSaveDelegate OnSaved)
{
	if (auto SaveGameInstance = Cast<UWorldSaveGame>(UGameplayStatics::CreateSaveGameObject(StaticClass())))
	{
		SaveGameInstance->Seed = NewSeed;

		const auto SavePath = FPaths::Combine(FPaths::ProjectSavedDir(), FString("Worlds"), SaveName, FString("World.sav"));
		
		AsyncTask(ENamedThreads::AnyThread, [SaveGameInstance, SavePath, OnSaved]()
		{
			TArray<uint8> Characters;
			UGameplayStatics::SaveGameToMemory(SaveGameInstance, Characters);

			FFileHelper::SaveArrayToFile(Characters, *SavePath);

			AsyncTask(ENamedThreads::GameThread, [&]()
			{
				OnSaved.ExecuteIfBound();
			});
		});
	}
}

void UWorldSaveGame::DeleteWorld(const FString& SaveName)
{ 
	const auto& WorldPath = FPaths::Combine(FPaths::ProjectSavedDir(), FString("Worlds"), SaveName);

	if (FPaths::DirectoryExists(WorldPath))
		IFileManager::Get().DeleteDirectory(*WorldPath, true, true);
}

bool UWorldSaveGame::TryToLoadWorld(const FString& SaveName, float& NewSeed)
{
	const auto& FilePath = FPaths::Combine(FPaths::ProjectSavedDir(), FString("Worlds"), SaveName, FString("World.sav"));
	
	if (!FPaths::FileExists(FilePath))
		return false;
	
	TArray<uint8> Characters;
	
	if (!FFileHelper::LoadFileToArray(Characters, *FilePath))
		return false;

	const auto& SaveGame = Cast<UWorldSaveGame>(UGameplayStatics::LoadGameFromMemory(Characters));
	if (!SaveGame)
		return false;

	NewSeed = SaveGame->Seed;
	return true;
}
