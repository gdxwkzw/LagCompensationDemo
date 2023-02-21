// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "UI_Ping.generated.h"

/**
 * 
 */
UCLASS()
class LAGCOMPENSATIONDEMO_API UUI_Ping : public UUserWidget
{
	GENERATED_BODY()
public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PingText;

	void SetHUDPing(float Ping);
};
