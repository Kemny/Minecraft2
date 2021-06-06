// Fill out your copyright notice in the Description page of Project Settings.


#include "StringFunctionLibrary.h"

bool UStringFunctionLibrary::IsEmptyOrWhitespace(const FString& Text)
{
	bool bOnlyWhitespace = true;
	for (const auto& Character : Text)
	{
		if (Character != TEXT(' '))
		{
			bOnlyWhitespace = false;
			break;
		}
	}
	
	if (bOnlyWhitespace)
		return true;
	
	return Text.IsEmpty();
}
