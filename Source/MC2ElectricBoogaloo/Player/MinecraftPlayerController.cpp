// Fill out your copyright notice in the Description page of Project Settings.

#include "MinecraftPlayerController.h"
#include "MinecraftCharacter.h"
#include "Camera/CameraComponent.h"

void AMinecraftPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const auto World = GetWorld();
	const auto TargetPawn = Cast<AMinecraftCharacter>(GetPawn());
	if (!World || !TargetPawn)
		return;
	
	const auto TargetCamera = TargetPawn->GetCamera();
	
	if (!TargetCamera)
		return;

	//const FName TraceTag("MyTraceTag");
	//World->DebugDrawTraceTag = TraceTag;
	
	FHitResult Hit;
	FCollisionQueryParams Params;
	//Params.TraceTag = TraceTag;
	Params.bTraceComplex = true;
	
	if (World->LineTraceSingleByObjectType(Hit, TargetCamera->GetComponentLocation(), TargetCamera->GetComponentLocation() + TargetCamera->GetForwardVector() * TraceRange,  ECC_GameTraceChannel1, Params))
	{
		SelectedPosition.Position = Hit.ImpactPoint;
		SelectedPosition.Normal = Hit.ImpactNormal;
	}
	else
	{
		SelectedPosition.Position = {0,0,-100};
	}
}
