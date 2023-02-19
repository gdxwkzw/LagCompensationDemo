// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "DemoHUD.generated.h"

/**
 * 
 */
UCLASS()
class LAGCOMPENSATIONDEMO_API ADemoHUD : public AHUD
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	class UTexture2D* Texture;

	virtual void DrawHUD() override;
};
