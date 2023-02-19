// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UCLASS()
class LAGCOMPENSATIONDEMO_API AWeapon : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWeapon();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere)
	class USceneComponent* RootScene;

	UPROPERTY(EditAnywhere)
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(EditAnywhere)
	UParticleSystem* BeamParticles;

	UPROPERTY(EditAnywhere)
	UParticleSystem* MuzzleFlash;

	UPROPERTY(EditAnywhere)
	class USoundCue* FireSoundCue;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:
	void Fire();

	void LocalFire(FVector TraceEnd);

	UFUNCTION(Server, Reliable)
	void ServerFire(FVector TraceEnd);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(FVector TraceEnd);

	UPROPERTY(EditAnywhere, Category = Weapon)
	bool bUseServerSideRewind = false;

	void HitConfirmed(FVector TraceEnd, class ALagCompensationDemoCharacter* HitCharacter);
private:
	FHitResult TraceEndResult;
	void TraceUnderCrosshairs(FHitResult& TraceHitResult);
};