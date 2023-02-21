// Copyright Epic Games, Inc. All Rights Reserved.

#include "LagCompensationDemoGameMode.h"
#include "LagCompensationDemoCharacter.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

ALagCompensationDemoGameMode::ALagCompensationDemoGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPerson/Blueprints/BP_ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}

void ALagCompensationDemoGameMode::RequestRespawn(ACharacter* RespawnedCharacter, AController* RespawnedController)
{
	if(RespawnedCharacter)
	{
		RespawnedCharacter->Reset();
		RespawnedCharacter->Destroy();
	}

	if(RespawnedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 SelectedIndex = FMath::RandRange(0, PlayerStarts.Num()-1);
		RestartPlayerAtPlayerStart(RespawnedController, PlayerStarts[SelectedIndex]);
	}
}
