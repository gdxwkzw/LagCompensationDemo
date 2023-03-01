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
	virtual void NativeOnInitialized() override;
	
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* PingText;

	UPROPERTY(meta = (BindWidget))
	class UCheckBox* UseLagCompensationCheckBox;

	void SetHUDPing(float Ping);

	UFUNCTION()
	void OnLagCompensationCheckBoxChange(bool bIsChecked);
};
