// Fill out your copyright notice in the Description page of Project Settings.

#include "MinecraftPlayerController.h"

void AMinecraftPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const auto World = GetWorld();
	if (!World)
		return;

	FHitResult Hit;
	if (World->LineTraceSingleByObjectType(Hit, GetPawn()->GetActorLocation(), GetPawn()->GetActorForwardVector() * 1000,  ECC_GameTraceChannel1))
	{
		UE_LOG(LogTemp, Log, TEXT("{%f} [%f,%f,%f]"), Hit.Distance, Hit.ImpactPoint.X, Hit.ImpactPoint.Y, Hit.ImpactPoint.Z)
		//Hit.ImpactNormal
	}
}
