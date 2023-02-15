// Copyright Epic Games, Inc. All Rights Reserved.

#include "LagCompensationDemoGameMode.h"
#include "LagCompensationDemoCharacter.h"
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
