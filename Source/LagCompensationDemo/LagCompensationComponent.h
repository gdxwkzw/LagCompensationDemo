// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LagCompensationDemoCharacter.h"
#include "Components/ActorComponent.h"
#include "LagCompensationComponent.generated.h"

USTRUCT()
struct FFramePackage
{
	GENERATED_BODY()

	UPROPERTY()
	ALagCompensationDemoCharacter* Character = nullptr;

	UPROPERTY()
	float Time = 0.f;

	UPROPERTY()
	FVector HitBoxLocation;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LAGCOMPENSATIONDEMO_API ULagCompensationComponent : public UActorComponent
{
	GENERATED_BODY()
public:	
	// Sets default values for this component's properties
	ULagCompensationComponent();

	void CacheFramePackage(ALagCompensationDemoCharacter* Character, FFramePackage& Package);
	void SaveFramePackage();
	void ShowFramePackage(const FFramePackage& Package);
	FFramePackage InterpBetweenFrames(const FFramePackage& OlderFrame, const FFramePackage& YoungerFrame, float HitTime);
	FFramePackage GetFrameToCheck(ALagCompensationDemoCharacter* HitCharacter, float HitTime);
	UFUNCTION(Server, Reliable)
	void ServerHitComfirm(
		ALagCompensationDemoCharacter* HitCharacter,
		const FVector_NetQuantize& TraceStart,
		const FVector_NetQuantize& TraceEnd,
		float HitTime
	);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

private:
	UPROPERTY()
	ALagCompensationDemoCharacter* DemoCharacter;

	TDoubleLinkedList<FFramePackage> FrameHistory;

	UPROPERTY(EditAnywhere, Category = LagCompensation)
	float MaxRecordTime = 4.f;
};