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
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
public:
	void Fire();

	UPROPERTY(EditAnywhere, Category = Weapon)
	bool bUseServerSideRewind = false;
	
};
