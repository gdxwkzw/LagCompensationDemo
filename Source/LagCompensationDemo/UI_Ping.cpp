// Fill out your copyright notice in the Description page of Project Settings.


#include "UI_Ping.h"

#include "Components/TextBlock.h"

void UUI_Ping::SetHUDPing(float Ping)
{
	if(PingText)
	{
		FString PingString = FString::Printf(TEXT("%.0f"), Ping * 1000); // Seconds to Milliseconds 秒转毫秒
		PingText->SetText(FText::FromString(PingString));
	}
}
