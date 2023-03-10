// Copyright Epic Games, Inc. All Rights Reserved.

#include "LagCompensationDemoCharacter.h"

#include "DemoPlayerController.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "LagCompensationComponent.h"
#include "LagCompensationDemoGameMode.h"
#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"


//////////////////////////////////////////////////////////////////////////
// ALagCompensationDemoCharacter

ALagCompensationDemoCharacter::ALagCompensationDemoCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm


	// LagCompensation component
	LagCompensationComponent = CreateDefaultSubobject<ULagCompensationComponent>(TEXT("LagCompensationComponent"));

	//LagCompensation hitBox
	LagCompensationHitBox = CreateDefaultSubobject<UCapsuleComponent>(TEXT("LagCompensationHitBox"));
	LagCompensationHitBox->SetupAttachment(GetMesh());
	LagCompensationHitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named ThirdPersonCharacter (to avoid direct content references in C++)

	bUseControllerRotationYaw = true;
}



void ALagCompensationDemoCharacter::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	//Add Input Mapping Context
	ADemoPlayerController* PlayerController = Cast<ADemoPlayerController>(Controller);
	if (PlayerController)
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	if(HasAuthority() && GetMesh())
	{
		if(const USkeletalMeshSocket* HandSocket = GetMesh()->GetSocketByName(FName("WeaponSocket")))
		{
			if(WeaponClass)
			{
				Weapon = GetWorld()->SpawnActor<AWeapon>(WeaponClass);
				if(Weapon)
				{
					HandSocket->AttachActor(Weapon, GetMesh());
					Weapon->SetOwner(this);
					if(PlayerController)
					{
						Weapon->bUseLagCompensation = PlayerController->bUseLagCompensation;
					}
				}
			}
			
		}
	}
}

void ALagCompensationDemoCharacter::OnRep_IsDeath()
{
	if(bIsDeath)
	{
		Die();
	}
}

void ALagCompensationDemoCharacter::RespawnTimerFinished()
{
	if(ALagCompensationDemoGameMode* DemoGameMode = GetWorld()->GetAuthGameMode<ALagCompensationDemoGameMode>())
	{
		DemoGameMode->RequestRespawn(this, Controller);
	}
	
}

void ALagCompensationDemoCharacter::SetLagCompensationHitBox(FVector Location)
{
	LagCompensationHitBox->SetWorldLocation(Location);
}

void ALagCompensationDemoCharacter::Die()
{
	bEnableMovement = false;
	if(HasAuthority())
	{
		bIsDeath = true;
		GetWorldTimerManager().SetTimer(
			RespawnTimer,
			this,
			&ThisClass::RespawnTimerFinished,
			5.f
		);
	}
	
	if(GetMovementComponent())
	{
		GetMovementComponent()->StopMovementImmediately();
	}
}

void ALagCompensationDemoCharacter::PlayHitReact()
{
	if(!GetMesh()) return;
	if(UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if(HitReactMontage)
		{
			AnimInstance->Montage_Play(HitReactMontage);
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// Input

void ALagCompensationDemoCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		//Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		//Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ALagCompensationDemoCharacter::Move);

		//Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ALagCompensationDemoCharacter::Look);

		//Fire
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Triggered, this, &ALagCompensationDemoCharacter::FireButtonPressed);

		//Show Mouse Cursor
		EnhancedInputComponent->BindAction(ShowCursorAction, ETriggerEvent::Triggered, this, &ALagCompensationDemoCharacter::ShowCursorButtonPressed);

		//Hide Mouse Cursor
		EnhancedInputComponent->BindAction(HideCursorAction, ETriggerEvent::Triggered, this, &ALagCompensationDemoCharacter::ShowCursorButtonRelease);
	}

}

void ALagCompensationDemoCharacter::Move(const FInputActionValue& Value)
{
	if(!bEnableMovement) return;
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	
		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void ALagCompensationDemoCharacter::Look(const FInputActionValue& Value)
{
	if(!bEnableMovement) return;
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ALagCompensationDemoCharacter::FireButtonPressed()
{
	if(!bEnableMovement) return;
	if(Weapon)
	{
		Weapon->Fire();
	}
}

void ALagCompensationDemoCharacter::ShowCursorButtonPressed()
{
	if(ADemoPlayerController* PlayerController = Cast<ADemoPlayerController>(Controller))
	{
		PlayerController->SetShowMouseCursor(true);
	}
}

void ALagCompensationDemoCharacter::ShowCursorButtonRelease()
{
	if(ADemoPlayerController* PlayerController = Cast<ADemoPlayerController>(Controller))
	{
		PlayerController->SetShowMouseCursor(false);
	}
}

void ALagCompensationDemoCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ALagCompensationDemoCharacter, Weapon);
	DOREPLIFETIME(ALagCompensationDemoCharacter, bIsDeath);
}

void ALagCompensationDemoCharacter::Destroyed()
{
	if(Weapon)
	{
		Weapon->Destroy();
	}
	
	Super::Destroyed();
}
