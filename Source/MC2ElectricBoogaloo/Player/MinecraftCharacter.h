// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MinecraftCharacter.generated.h"

class UCameraComponent;
UCLASS()
class MC2ELECTRICBOOGALOO_API AMinecraftCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AMinecraftCharacter();
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	UCameraComponent* GetCamera() const { return FirstPersonCameraComponent; }
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FirstPersonCameraComponent;

	UFUNCTION()
	void MoveForward(float Axis);
	UFUNCTION()
	void MoveRight(float Axis);

};
