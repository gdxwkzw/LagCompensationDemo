// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "DemoAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class LAGCOMPENSATIONDEMO_API UDemoAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

protected:
	UPROPERTY()
	class ALagCompensationDemoCharacter* DemoCharacter;
	UPROPERTY(BlueprintReadOnly)
	float Speed = 0.f;
	UPROPERTY(BlueprintReadOnly)
	bool bIsDeath = false;
	UPROPERTY(BlueprintReadOnly)
	bool bIsInAir = false;
public:
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
};
