// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NoiseGenerator.generated.h"

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MC2ELECTRICBOOGALOO_API UNoiseGenerator : public UActorComponent
{
	GENERATED_BODY()

public:
	UNoiseGenerator();
	virtual void BeginPlay() override;

	/*
	TODO Set actor with noises to position, generato to noise render target, return float array of R channel
	UFUNCTION(BlueprintCallable)
	GenerateNoise
	*/
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInstance* NoiseMaterial;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UTextureRenderTarget2D* NoiseRenderTarget;
};
