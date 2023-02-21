// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "LagCompensationDemoGameMode.generated.h"

UCLASS(minimalapi)
class ALagCompensationDemoGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ALagCompensationDemoGameMode();
	void RequestRespawn(ACharacter* RespawnedCharacter, AController* Respawned);
};



