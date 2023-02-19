// Fill out your copyright notice in the Description page of Project Settings.


#include "DemoAnimInstance.h"

#include "LagCompensationDemoCharacter.h"
#include "GameFramework/PawnMovementComponent.h"

void UDemoAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	DemoCharacter = Cast<ALagCompensationDemoCharacter>(TryGetPawnOwner());
}

void UDemoAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);
	DemoCharacter = DemoCharacter == nullptr ? Cast<ALagCompensationDemoCharacter>(TryGetPawnOwner()) : DemoCharacter;
	if(DemoCharacter == nullptr) return;
	
	Speed = DemoCharacter->GetSpeed();
	bIsDeath = DemoCharacter->IsDeath();
	if(DemoCharacter->GetMovementComponent())
	{
		bIsInAir = DemoCharacter->GetMovementComponent()->IsFalling();
	}
	
}
