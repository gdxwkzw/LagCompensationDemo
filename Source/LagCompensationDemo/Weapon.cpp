// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"

#include "DemoPlayerController.h"
#include "EngineUtils.h"
#include "LagCompensationComponent.h"
#include "LagCompensationDemoCharacter.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Components/CapsuleComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"
#include "Particles/ParticleSystemComponent.h"
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

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AWeapon, bUseLagCompensation);
}

void AWeapon::Fire()
{
	TraceUnderCrosshairs(TraceEndResult);
	LocalFire(TraceEndResult.Location);
	if(!bUseLagCompensation)
	{
		ServerFire(TraceEndResult.Location);
	}
	else
	{
		ALagCompensationDemoCharacter* HitCharacter = Cast<ALagCompensationDemoCharacter>(TraceEndResult.GetActor());
		if(ALagCompensationDemoCharacter* OwnerCharacter = GetOwner<ALagCompensationDemoCharacter>())
		{
			if(ADemoPlayerController* OwnerController = OwnerCharacter->GetController<ADemoPlayerController>())
			{
				ServerFireWithLagCompensation(
					HitCharacter,
					TraceEndResult.Location,
					OwnerController->GetServerTime() - OwnerController->SingleTripTime
				);
			}
		}
		
	}
	
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

	FVector BeamEnd = TraceEnd;
	DrawDebugLine(GetWorld(), SocketTransform.GetLocation(), TraceEnd, FColor::Green, false, 10.f);
	if(HitResult.bBlockingHit)
	{
		BeamEnd = HitResult.Location;
		if(MuzzleFlash)
		{
			UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MuzzleFlash, HitResult.ImpactPoint, HitResult.ImpactNormal.Rotation());
		}
			
		if(ALagCompensationDemoCharacter* HitCharacter = Cast<ALagCompensationDemoCharacter>(HitResult.GetActor()))
		{
			if(bClientDrawDebugCapsule && HitCharacter->GetCapsuleComponent())
			{
				DrawDebugCapsule(
					GetWorld(),
					HitCharacter->GetCapsuleComponent()->GetComponentLocation(),
					HitCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
					HitCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius(),
					HitCharacter->GetCapsuleComponent()->GetComponentRotation().Quaternion(),
					FColor::Orange,
					false,
					4.f)
				;
			}
			
			HitCharacter->PlayHitReact();
		}
	}

	if(BeamParticles)
	{
		UParticleSystemComponent* Beam = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BeamParticles, SocketTransform);
		if(Beam)
		{
			Beam->SetVectorParameter(FName("Target"), BeamEnd);
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

	if(WeaponMesh == nullptr || bUseLagCompensation) return;
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
			HitCharacter->Die();
		}
	}

	/** When lag compensation is not used,
	 * the position of the character on the server
	 * at the time of hit confimed is displayed.
	 * 不使用延迟补偿时展示做判定时其他角色在服务器上的位置 */
	if(bClientDrawDebugCapsule)
	{
		for(TActorIterator<ALagCompensationDemoCharacter> Iterator(GetWorld()); Iterator; ++Iterator)
		{
			ALagCompensationDemoCharacter* OtherCharacter = *Iterator;
			if(OtherCharacter && OtherCharacter != GetOwner())
			{
				MulticastDrawDebugCapsule(
					OtherCharacter->GetCapsuleComponent()->GetComponentLocation(),
					OtherCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight(),
					OtherCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius(),
					OtherCharacter->GetCapsuleComponent()->GetComponentRotation().Quaternion(),
					FColor::Red,
					false,
					10.f
				);
			}
		}
	}
	/** ---------------------------------------------- */
}

void AWeapon::ServerFireWithLagCompensation_Implementation(ALagCompensationDemoCharacter* HitCharacter, FVector TraceEnd, float HitTime)
{
	MulticastFire(TraceEnd);

	if(WeaponMesh == nullptr || HitCharacter == nullptr || !bUseLagCompensation) return;
	const USkeletalMeshSocket* MuzzleFlashSocket = WeaponMesh->GetSocketByName(FName("MuzzleFlash"));
	if(MuzzleFlashSocket == nullptr) return;
	FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(WeaponMesh);

	if(HitCharacter->GetLagCompensationComponent())
	{
		/**  Use the lag compensation component to comfirm hit with lag compensation
		 *使用延迟补偿组件做带延迟补偿的射击判定
		 */
		HitCharacter->GetLagCompensationComponent()->ServerHitComfirm(
			HitCharacter,
			SocketTransform.GetLocation(),
			TraceEnd,
			HitTime
		);
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
			TraceHitResult.Location = End;
		}
	}
}

void AWeapon::MulticastDrawDebugCapsule_Implementation(const FVector_NetQuantize& Center, float HalfHeight, float Radius,
	const FQuat& Rotation, const FColor& Color, bool bPersistentLines, float LiftTime)
{
	DrawDebugCapsule(
		GetWorld(),
		Center,
		HalfHeight,
		Radius,
		Rotation,
		Color,
		bPersistentLines,
		LiftTime
	);
}

// Called every frame
void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
}

