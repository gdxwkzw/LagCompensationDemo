// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "LagCompensationDemoCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#define TRACE_LENGTH 99999.f

// Sets default values
AWeapon::AWeapon()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	RootScene = CreateDefaultSubobject<USceneComponent>(TEXT("RootScene"));
	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	RootComponent = RootScene;
	WeaponMesh->SetupAttachment(RootScene);
	SetReplicates(true);
}

// Called when the game starts or when spawned
void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}

void AWeapon::Fire()
{
	TraceUnderCrosshairs(TraceEndResult);
	LocalFire(TraceEndResult.Location);
	ServerFire(TraceEndResult.Location);
}

void AWeapon::LocalFire(FVector TraceEnd)
{
	if(WeaponMesh == nullptr) return;
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash"));
	if(MuzzleFlashSocket == nullptr) return;
	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);
	if(FireSoundCue)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, FireSoundCue, SocketTransform.GetLocation());
	}
	
	if(MuzzleFlash)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, SocketTransform);
	}

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		SocketTransform.GetLocation(),
		TraceEnd,
		ECollisionChannel::ECC_Pawn
	);

	if(HitResult.bBlockingHit)
	{
		if(MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
		}
			
		if(ALagCompensationDemoCharacter* HitCharacter = Cast<ALagCompensationDemoCharacter>(HitResult.GetActor()))
		{
			HitCharacter->PlayHitReact();
		}
	}
}

void AWeapon::MulticastFire_Implementation(FVector TraceEnd)
{
	ALagCompensationDemoCharacter* DemoCharacter = Cast<ALagCompensationDemoCharacter>(GetOwner());
	if(DemoCharacter && !DemoCharacter->IsLocallyControlled())
	{
		LocalFire(TraceEnd);
	}
}

void AWeapon::ServerFire_Implementation(FVector TraceEnd)
{
	MulticastFire(TraceEnd);
	if(WeaponMesh == nullptr) return;
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash"));
	if(MuzzleFlashSocket == nullptr) return;
	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);

	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		SocketTransform.GetLocation(),
		TraceEnd,
		ECollisionChannel::ECC_Pawn
	);

	if(HitResult.bBlockingHit)
	{
		if(ALagCompensationDemoCharacter* HitCharacter = Cast<ALagCompensationDemoCharacter>(HitResult.GetActor()))
		{
			if(!bUseServerSideRewind)
			{
				HitConfirmed(TraceEnd, HitCharacter);
			}
		}
	}
}

void AWeapon::HitConfirmed(FVector TraceEnd, ALagCompensationDemoCharacter* HitCharacter)
{
	if(WeaponMesh == nullptr || HitCharacter == nullptr) return;
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash"));
	if(MuzzleFlashSocket == nullptr) return;
	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);
	FHitResult HitResult;
	GetWorld()->LineTraceSingleByChannel(
		HitResult,
		SocketTransform.GetLocation(),
		TraceEnd,
		ECollisionChannel::ECC_Pawn
	);

	if(HitResult.bBlockingHit)
	{
		if(HitResult.GetActor() == HitCharacter && HasAuthority())
		{
			HitCharacter->Die();
		}
	}
}

void AWeapon::TraceUnderCrosshairs(FHitResult& TraceHitResult)
{
	FVector2D ViewPortSize;
	if(GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewPortSize);
	}

	FVector2D CrosshairLocation(ViewPortSize.X / 2.f, ViewPortSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);

	if(bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;

		if(Owner)
		{
			float DistanceToCharacter = (Owner->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (DistanceToCharacter + 100.f);
		}
		
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECollisionChannel::ECC_Visibility
			);
		if(!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
	}
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

