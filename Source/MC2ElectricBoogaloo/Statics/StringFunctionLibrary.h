// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "StringFunctionLibrary.generated.h"


UCLASS()
class MC2ELECTRICBOOGALOO_API UStringFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintPure)
	static bool IsEmptyOrWhitespace(const FString& Text);
};
