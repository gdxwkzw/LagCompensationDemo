// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "DemoPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class LAGCOMPENSATIONDEMO_API ADemoPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	float ClientServerDelta = 0.f;// Difference between client and server time
	float SingleTripTime = 0.f;

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientRequestServerTime(float TimeOfClientRequest, float TimeServerReceivedClientRequest);
	
	float GetServerTime();
	virtual void ReceivedPlayer() override;

	UPROPERTY(EditAnywhere)
	float TimeSyncFrequency = 5.f;
	float TimeSyncRunningTime = 0.f;
	void CheckTimeSync(float DeltaTime);
	virtual void Tick(float DeltaSeconds) override;
};
