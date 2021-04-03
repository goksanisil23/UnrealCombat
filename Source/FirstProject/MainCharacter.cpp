// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"

#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/World.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Kismet/KismetMathLibrary.h"
#include "Enemy.h"
#include "MainPlayerController.h"


// Sets default values
AMainCharacter::AMainCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create camera boom (pulls towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 600.f; // Camera follows at this distance
	CameraBoom->bUsePawnControlRotation = true; // rotate arm based on controller

	// Create FollowCamera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	// Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false;

	BaseTurnRate = 65.f;
	BaseLookupRate = 65.f;

	// set size for collision capsule
	GetCapsuleComponent()->SetCapsuleSize(48.f, 150.f);

	// Don't rotate when the controller rotates, let that just affect the camera
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;

	// character moves in the direction of input
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 540.f, 0.f); // only for yaw
	GetCharacterMovement()->JumpZVelocity = 650.f;
	GetCharacterMovement()->AirControl = 0.2f;

	MaxHealth = 100.0f;
	Health = 65.0f;
	MaxStamina = 150.0f;
	Stamina = 120.0f;
	Coins = 0;

	RunningSpeed = 650.0f;
	SprintingSpeed = 950.0f;

	bShiftKeyDown = false;
	bLMBDown = false;

	// Initialize Enums
	MovementStatus = EMovementStatus::EMS_Normal;
	StaminaStatus = EStaminaStatus::ESS_Normal;

	StaminaDrainRate = 25.0f;
	MinSprintStamina = 50.0f;

	InterpSpeed = 15.0f;
	bInterpToEnemy = false;

	bHasCombatTarget = false;

	bMovingRight = false;
	bMovingForward = false;
}

// Called when the game starts or when spawned
void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();

	MainPlayerController = Cast<AMainPlayerController>(GetController());
	
}

FRotator AMainCharacter::GetLookAtRotationYaw(FVector Target) {
	// Finds the rotation required by this object, to be able look at a target location
	FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Target);
	FRotator LookAtRotationYaw(0.0f, LookAtRotation.Yaw, 0.0f);
	return LookAtRotation;
}

// Called every frame
void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(MovementStatus == EMovementStatus::EMS_Dead)
		return;
	
	float DeltaStamina = StaminaDrainRate * DeltaTime;
	
	switch (StaminaStatus) {
		case EStaminaStatus::ESS_Normal:
		if(bShiftKeyDown) {
			if(Stamina - DeltaStamina <= MinSprintStamina) {
				SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
				Stamina -= DeltaStamina;
			}
			else {
				Stamina -= DeltaStamina;
			}
			if (bMovingForward || bMovingRight) {
				SetMovementStatus(EMovementStatus::EMS_Sprinting);
			}			
			else {
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
		}
		else { // Shift key up
			if (Stamina + DeltaStamina >= MaxStamina) {
				Stamina = MaxStamina;
			}
			else {
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
		}
			break;
		case EStaminaStatus::ESS_BelowMinimum:
			if(bShiftKeyDown) {
				if (Stamina - DeltaStamina <= 0.0f) {
					SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
					Stamina = 0;
					SetMovementStatus(EMovementStatus::EMS_Normal);
				}
				else {
					Stamina -= DeltaStamina;
					if (bMovingForward || bMovingRight) {
						SetMovementStatus(EMovementStatus::EMS_Sprinting);
					}			
					else {
						SetMovementStatus(EMovementStatus::EMS_Normal);
					}					
				}
			}
			else { // Shift key up
				if (Stamina + DeltaStamina >= MinSprintStamina) {
					SetStaminaStatus(EStaminaStatus::ESS_Normal);
					Stamina += DeltaStamina;
				}
				else {
					Stamina += DeltaStamina;
				}
				SetMovementStatus(EMovementStatus::EMS_Normal);
			}
			break;
		case EStaminaStatus::ESS_Exhausted:
			if(bShiftKeyDown) {
				Stamina = 0;
			}
			else {
				SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovering);
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
			break;
		case EStaminaStatus::ESS_ExhaustedRecovering:
			if (Stamina + DeltaStamina >= MinSprintStamina) {
				Stamina += DeltaStamina;
				SetStaminaStatus(EStaminaStatus::ESS_Normal); 
			}
			else {
				Stamina += DeltaStamina;
			}
			SetMovementStatus(EMovementStatus::EMS_Normal);
			break;
		default:
			break;
	}

	//  INterp to Enemy Logic
	if(bInterpToEnemy && CombatTarget) {
		FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
		FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);

		SetActorRotation(InterpRotation);
	}

	if(CombatTarget) {
		CombatTargetLocation = CombatTarget->GetActorLocation();
		if(MainPlayerController) {
			MainPlayerController->EnemyLocation = CombatTargetLocation;
		}
	}

}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	// Binding action only calls based on pressed/release
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMainCharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &AMainCharacter::StopJumping);

	// Binding action only calls based on pressed/release
	PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMainCharacter::ShiftKeyDown);
	PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMainCharacter::ShiftKeyUp);	

	// Binding action only calls based on pressed/release
	PlayerInputComponent->BindAction("LMB", IE_Pressed, this, &AMainCharacter::LMBDown);
	PlayerInputComponent->BindAction("LMB", IE_Released, this, &AMainCharacter::LMBUp);		

	// Bind axis calls the function every frame, based on the value
	PlayerInputComponent->BindAxis("MoveForward", this, &AMainCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainCharacter::MoveRight);
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &AMainCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUpRate", this, &AMainCharacter::LookupAtRate);

}

void AMainCharacter::MoveForward(float Value) {

	bMovingForward = false;
	if((Controller != nullptr) && (Value != 0.0f) && !bAttacking && (MovementStatus != EMovementStatus::EMS_Dead)) {
		// Find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);

		bMovingForward = true;
	}

}

void AMainCharacter::MoveRight(float Value) {
	
	bMovingRight = false;
	if((Controller != nullptr) && (Value != 0.0f) && !bAttacking && (MovementStatus != EMovementStatus::EMS_Dead)) {
		// Find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		AddMovementInput(Direction, Value);

		bMovingRight = true;
	}
}

void AMainCharacter::TurnAtRate(float Rate) {
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}


void AMainCharacter::LookupAtRate(float Rate) {
	AddControllerPitchInput(Rate * BaseLookupRate * GetWorld()->GetDeltaSeconds());
}

void AMainCharacter::LMBDown() {
	bLMBDown = true;

	if(MovementStatus == EMovementStatus::EMS_Dead)
		return;

	// Choosing to equip the collided weapon
	if(ActiveOverlappingItem) {
		AWeapon* Weapon = Cast<AWeapon>(ActiveOverlappingItem);
		if(Weapon) {
			Weapon->Equip(this);
			SetActiveOverlappingItem(nullptr); // once picked, no more overlapping item left
		}
	}
	else { // if weapon is picked up, LMB is attacking
		if(EquippedWepon) {
			Attack();
		}
	}
}

void AMainCharacter::LMBUp() {
	bLMBDown = false;
}


void AMainCharacter::DecrementHealth(float Amount) {
	if (Health - Amount >= 0.0f) {
		Health -= Amount;
	}
	else {
		Health -= Amount;
		Die();
	}
}

void AMainCharacter::IncrementCoins(int32 Amount) {
	Coins += Amount;
}
void AMainCharacter::IncrementHealth(float Amount) {
	if(Health + Amount >= MaxHealth) {
		Health = MaxHealth;
	}
	else {
		Health += Amount;
	}
}

void AMainCharacter::Die() {
	if (MovementStatus == EMovementStatus::EMS_Dead) // Against multiple enemies attacking and multiple deaths
		return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && CombatMontage) {
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}
	SetMovementStatus(EMovementStatus::EMS_Dead);
}

void AMainCharacter::SetMovementStatus(EMovementStatus Status) {

	MovementStatus = Status;

	if (Status == EMovementStatus::EMS_Sprinting) {
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else {
		GetCharacterMovement()->MaxWalkSpeed = RunningSpeed;
	}

}

void AMainCharacter::ShiftKeyDown(){
	bShiftKeyDown = true;
}

void AMainCharacter::ShiftKeyUp(){
	bShiftKeyDown = false;
}

void AMainCharacter::ShowPickupLocations() {
	for(int32 i = 0; i < PickupLocations.Num(); i++) {
		// Draw a debug sphere at the coin pickup locations
		UKismetSystemLibrary::DrawDebugSphere(this, PickupLocations[i], 25.0f, 12, FLinearColor::Green, 10.0, 1.0);
	}
}

void AMainCharacter::SetEquippedWeapon(AWeapon* WeaponToSet) {
	if (EquippedWepon) {
		EquippedWepon->Destroy();
	}
	EquippedWepon = WeaponToSet;
}

void AMainCharacter::Attack() {
	if(!bAttacking && MovementStatus != EMovementStatus::EMS_Dead) {
		bAttacking = true;
		SetInerpToEnemy(true);

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();

		if(AnimInstance && CombatMontage) {

			int32 Section = FMath::RandRange(0, 1);
			switch(Section) {
				case 0:
					AnimInstance->Montage_Play(CombatMontage, 2.2f);
					AnimInstance->Montage_JumpToSection(FName("Attack_1"), CombatMontage);
					break;
				case 1:
					AnimInstance->Montage_Play(CombatMontage, 1.8f);
					AnimInstance->Montage_JumpToSection(FName("Attack_2"), CombatMontage);
					break;
				default:;
			}

		}
	}
}


void AMainCharacter::AttackEnd() {
	bAttacking = false;
	SetInerpToEnemy(false);

	if(bLMBDown) {
		Attack();	
	}
}

void AMainCharacter::PlaySwingSound(){
	if(EquippedWepon->SwingSoundCue) {
		UGameplayStatics::PlaySound2D(this, EquippedWepon->SwingSoundCue);
	}
	
}

void AMainCharacter::SetInerpToEnemy(bool Interp) {
	bInterpToEnemy = Interp;
}

float AMainCharacter::TakeDamage(float DamageAmount,struct FDamageEvent const & DamageEvent, 
					class AController * EventInstigator, AActor * DamageCauser) {

	if (Health - DamageAmount >= 0.0f) {
		Health -= DamageAmount;
	}
	else {
		Health -= DamageAmount;
		Die();
		if (DamageCauser) {
			AEnemy* Enemy = Cast<AEnemy>(DamageCauser);
			if(Enemy){
				Enemy->bHasValidTarget = false; // after the main char dies, enemies have no more valid target
			}
		}		
	}

	return DamageAmount;

}

void AMainCharacter::DeathEnd() {
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;
}

void AMainCharacter::Jump() {
	if(MovementStatus != EMovementStatus::EMS_Dead) {
		Super::Jump();
	}
}

void AMainCharacter::UpdateCombatTarget() {
	TArray<AActor*> OverlappingActors;
	GetOverlappingActors(OverlappingActors, EnemyFilter);

	if(OverlappingActors.Num() == 0) {
		if(MainPlayerController){
			MainPlayerController->RemoveEnemyHealthBar();
		}
		return;
	}

	AEnemy* ClosestEnemy = Cast<AEnemy>(OverlappingActors[0]);
	if(ClosestEnemy){

		FVector Location = GetActorLocation();
		float MinDistance = (ClosestEnemy->GetActorLocation() - Location).Size();
	
		for(auto Actor: OverlappingActors) {
			AEnemy* Enemy = Cast<AEnemy>(Actor);
			if(Enemy) {
				float DistanceToActor = (Enemy->GetActorLocation() - Location).Size();
				if(DistanceToActor < MinDistance) {
					MinDistance = DistanceToActor;
					ClosestEnemy = Enemy;
				}
			}

		}

		if(MainPlayerController){
			MainPlayerController->DisplayEnemyHealthBar();
		}
		SetCombatTarget(ClosestEnemy);
		bHasCombatTarget = true;
	}
	
}