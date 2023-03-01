// Fill out your copyright notice in the Description page of Project Settings.


#include "DemoPlayerController.h"

#include "LagCompensationDemoCharacter.h"
#include "UI_Ping.h"
#include "Weapon.h"
#include "Net/UnrealNetwork.h"


void ADemoPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	float ServerTimeOfReceipt = GetWorld()->GetTimeSeconds();
	ClientRequestServerTime(TimeOfClientRequest, ServerTimeOfReceipt);
}

void ADemoPlayerController::ClientRequestServerTime_Implementation(float TimeOfClientRequest,
	float TimeServerReceivedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	SingleTripTime = 0.5f * RoundTripTime;
	float CurrentServerTime = TimeServerReceivedClientRequest + RoundTripTime;
	ClientServerDelta = GetWorld()->GetTimeSeconds() - CurrentServerTime;

	//Update PingHUD
	DemoHUD = DemoHUD == nullptr ? GetHUD<ADemoHUD>() : DemoHUD;
	if(DemoHUD && DemoHUD->PingUI)
	{
		DemoHUD->PingUI->SetHUDPing(SingleTripTime);
	}
}

float ADemoPlayerController::GetServerTime()
{
	if(HasAuthority()) return GetWorld()->GetTimeSeconds();

	return GetWorld()->GetTimeSeconds() - ClientServerDelta;
}

void ADemoPlayerController::ReceivedPlayer()
{
	Super::ReceivedPlayer();
	if(IsLocalController())
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
	}
}

void ADemoPlayerController::CheckTimeSync(float DeltaTime)
{
	TimeSyncRunningTime += DeltaTime;
	if(IsLocalController() && TimeSyncRunningTime > TimeSyncFrequency)
	{
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
		TimeSyncRunningTime = 0.f;
	}
}

void ADemoPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	CheckTimeSync(DeltaSeconds);
}

void ADemoPlayerController::BeginPlay()
{
	Super::BeginPlay();

	DemoHUD = DemoHUD == nullptr ? GetHUD<ADemoHUD>() : DemoHUD;
	if(DemoHUD)
	{
		DemoHUD->AddPingUI();
	}
}

void ADemoPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ADemoPlayerController, bUseLagCompensation);
}

void ADemoPlayerController::ServerSetUseLagCompensation_Implementation(bool bUse)
{
	bUseLagCompensation = bUse;
	if(ALagCompensationDemoCharacter* ControlCharacter = GetPawn<ALagCompensationDemoCharacter>())
	{
		if(AWeapon* ControlWeapon = ControlCharacter->GetWeapon())
		{
			ControlWeapon->bUseLagCompensation = bUse;
		}
	}
	
}
