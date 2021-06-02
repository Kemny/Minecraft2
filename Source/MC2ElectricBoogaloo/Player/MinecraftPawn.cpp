#include "MinecraftPawn.h"

AMinecraftPawn::AMinecraftPawn()
{
	PrimaryActorTick.bCanEverTick = true;

}

void AMinecraftPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void AMinecraftPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AMinecraftPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

