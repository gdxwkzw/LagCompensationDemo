// Fill out your copyright notice in the Description page of Project Settings.

#include "LagCompensationComponent.h"

#include "LagCompensationDemoCharacter.h"
#include "Components/CapsuleComponent.h"
#define ECC_LagCompensationHitBox ECC_GameTraceChannel1

// Sets default values for this component's properties
ULagCompensationComponent::ULagCompensationComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

void ULagCompensationComponent::CacheFramePackage(ALagCompensationDemoCharacter* Character,
	FFramePackage& Package)
{
	if(Character)
	{
		Package.Time = GetWorld()->GetTimeSeconds();
		Package.Character = Character;
		Package.HitBoxLocation = Character->GetLagCompensationHitBox()->GetComponentLocation();
	}
}

void ULagCompensationComponent::SaveFramePackage()
{
	DemoCharacter = DemoCharacter == nullptr ? Cast<ALagCompensationDemoCharacter>(GetOwner()) : DemoCharacter;
	if(DemoCharacter)
	{
		FFramePackage ThisFrame;
		CacheFramePackage(DemoCharacter, ThisFrame);
		if(FrameHistory.Num() < 1)
		{
			FrameHistory.AddHead(ThisFrame);
		}
		else
		{
			float HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
			while(HistoryLength > MaxRecordTime && FrameHistory.Num() > 0)
			{
				FrameHistory.RemoveNode(FrameHistory.GetTail());
				HistoryLength = FrameHistory.GetHead()->GetValue().Time - FrameHistory.GetTail()->GetValue().Time;
			}
			FrameHistory.AddHead(ThisFrame);
		}
		//ShowFramePackage(ThisFrame);
	}
}

void ULagCompensationComponent::ShowFramePackage(const FFramePackage& Package)
{
	DemoCharacter = DemoCharacter == nullptr ? Cast<ALagCompensationDemoCharacter>(GetOwner()) : DemoCharacter;
	if(DemoCharacter)
	{
		DrawDebugCapsule(
			GetWorld(),
			Package.HitBoxLocation,
			DemoCharacter->GetLagCompensationHitBox()->GetScaledCapsuleHalfHeight(),
			DemoCharacter->GetLagCompensationHitBox()->GetScaledCapsuleRadius(),
			DemoCharacter->GetLagCompensationHitBox()->GetComponentRotation().Quaternion(),
			FColor::Red,
			false,
			MaxRecordTime
		);
	}
	
}

FFramePackage ULagCompensationComponent::InterpBetweenFrames(const FFramePackage& OlderFrame,
	const FFramePackage& YoungerFrame, float HitTime)
{
	const float Distance = YoungerFrame.Time - OlderFrame.Time;
	const float InterpFraction = FMath::Clamp((HitTime - OlderFrame.Time) / Distance, 0.f, 1.f);
	FFramePackage InterpFramePackage;
	InterpFramePackage.Character = OlderFrame.Character;
	InterpFramePackage.Time = HitTime;
	InterpFramePackage.HitBoxLocation = FMath::VInterpTo(OlderFrame.HitBoxLocation, YoungerFrame.HitBoxLocation, 1.f, InterpFraction);

	return InterpFramePackage;
}

FFramePackage ULagCompensationComponent::GetFrameToCheck(ALagCompensationDemoCharacter* HitCharacter, float HitTime)
{
	bool bReturn = HitCharacter == nullptr ||
		HitCharacter->GetLagCompensationComponent() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetHead() == nullptr ||
		HitCharacter->GetLagCompensationComponent()->FrameHistory.GetTail() == nullptr;
	if(bReturn) return FFramePackage();

	// Frame package that we check to verify a hit 待验证的帧记录
	FFramePackage FrameToCheck;
	FrameToCheck.Character = HitCharacter;
	bool bShouldInterpolate = true;

	// Frame history of the HitCharacter 该玩家在服务器的帧历史记录
	const TDoubleLinkedList<FFramePackage>& History = HitCharacter->GetLagCompensationComponent()->FrameHistory;
	const float OldestHistoryTime = History.GetTail()->GetValue().Time;
	const float NewestHistoryTime = History.GetHead()->GetValue().Time;

	if(OldestHistoryTime > HitTime)
	{
		return FFramePackage();
	}

	if(OldestHistoryTime == HitTime)
	{
		FrameToCheck = History.GetTail()->GetValue();
		bShouldInterpolate = false;
	}

	if(NewestHistoryTime <= HitTime)
	{
		FrameToCheck = History.GetHead()->GetValue();
		bShouldInterpolate = false;
	}

	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Older = History.GetHead();
	TDoubleLinkedList<FFramePackage>::TDoubleLinkedListNode* Younger = Older;

	// Match: OlderTime <= HitTime < YoungerTime 最终OlderTime和YoungerTime这两游标要匹配成这样
	while(Older->GetValue().Time > HitTime && Older->GetNextNode() != nullptr)
	{
		Older = Older->GetNextNode();	
	}
	
	if(Older->GetPrevNode() != nullptr)
	{
		Younger = Older->GetPrevNode();
	}

	if(Older->GetValue().Time == HitTime) // OlderTime == HitTime
	{
		FrameToCheck = Older->GetValue();
		bShouldInterpolate = false;
	}

	// OlderTime < HitTime < YoungerTime
	if(bShouldInterpolate)
	{
		FrameToCheck = InterpBetweenFrames(Older->GetValue(), Younger->GetValue(), HitTime);
	}

	return FrameToCheck;
}

void ULagCompensationComponent::ServerHitComfirm_Implementation(ALagCompensationDemoCharacter* HitCharacter,
                                                                const FVector_NetQuantize& TraceStart, const FVector_NetQuantize& TraceEnd, float HitTime)
{
	if(HitCharacter == nullptr) return;
	FFramePackage FrameToCheck = GetFrameToCheck(HitCharacter, HitTime);
	FFramePackage CurrentFrame;
	CacheFramePackage(HitCharacter, CurrentFrame);

	// Move the hitBox to the time of the check 临时把HitBox的位置回退到检查时间并启用碰撞检测
	HitCharacter->SetLagCompensationHitBox(FrameToCheck.HitBoxLocation);
	HitCharacter->GetLagCompensationHitBox()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	
	FHitResult ConfirmHitResult;
	GetWorld()->LineTraceSingleByChannel(
		ConfirmHitResult,
		TraceStart,
		TraceEnd,
		ECollisionChannel::ECC_LagCompensationHitBox
	);

	if(ConfirmHitResult.bBlockingHit)
	{
		// Reset hitbox 把HitBox位置挪回来并关闭碰撞检测
		HitCharacter->SetLagCompensationHitBox(CurrentFrame.HitBoxLocation);
		HitCharacter->GetLagCompensationHitBox()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

		HitCharacter->Die();
	}
}

// Called when the game starts
void ULagCompensationComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void ULagCompensationComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SaveFramePackage();
	// ...
}

