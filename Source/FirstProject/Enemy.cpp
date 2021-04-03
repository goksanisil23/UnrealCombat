// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "Components/SphereComponent.h"
#include "AIController.h"
#include "MainCharacter.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Components/BoxComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Components/CapsuleComponent.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"
#include "MainPlayerController.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// All character classes have their Root component as sphere component, sth else cannot be root, can only be attached to it
	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->InitSphereRadius(700.0f);
	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());	
	CombatSphere->InitSphereRadius(155.0f);

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetMesh(), FName("EnemySocket"));


	bOverlappingCombatSphere = false;

	Health = 75.0f;
	MaxHealth = 100.0f;
	Damage = 10.0f;

	AttackMinTime = 0.5f;
	AttackMaxTime = 1.5f;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	DeathDelay = 3.0f;

	bHasValidTarget = false;

}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AAIController>(GetController());
	
	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::AgroSphereOnOverlapEnd);

	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatSphereOnOverlapEnd);

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemy::CombatOnOverlapEnd);	

    CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); // only when animation is in "hitting" position, weapon will collide
    CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemy::AgroSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
							int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) {


	if(OtherActor && Alive()) {
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		if(MainChar) {
			MoveToTarget(MainChar);
		}
	}
}

void AEnemy::AgroSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
							UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {

	if (OtherActor) {
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		if(MainChar){
			bHasValidTarget = false;
			if (MainChar->CombatTarget == this) {
				// Out of hit range but still chasing (and not in the middle of attack)
				MainChar->SetCombatTarget(nullptr);
			}
			MainChar->UpdateCombatTarget();
			// Out of chasing range, stop
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);

			MainChar->bHasCombatTarget = false;

			if(AIController) {
				AIController->StopMovement();
			}
		}
	}


}

void AEnemy::CombatSphereOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
							int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult){

	if (OtherActor && Alive()) {
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		if(MainChar){
			UE_LOG(LogTemp, Warning, TEXT("CombatSphereOnOverlapBegin"));
			bHasValidTarget = true;
			MainChar->SetCombatTarget(this);
			MainChar->bHasCombatTarget = true;
			MainChar->UpdateCombatTarget();
			CombatTarget = MainChar;
			bOverlappingCombatSphere = true;
			float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
			GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
		}
	}

}

void AEnemy::CombatSphereOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
							UPrimitiveComponent* OtherComp, int32 OtherBodyIndex){

	if (OtherActor && OtherComp) {
		AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
		if(MainChar){

			bOverlappingCombatSphere = false;

			if(MainChar->MainPlayerController) {
				MainChar->MainPlayerController->DisplayEnemyHealthBar();
			}

			MoveToTarget(MainChar);
			CombatTarget = nullptr;

			if(MainChar->CombatTarget == this) {
				MainChar->SetCombatTarget(nullptr);
				MainChar->bHasCombatTarget = false;
				MainChar->UpdateCombatTarget();
			}

			if(MainChar->MainPlayerController){
				USkeletalMeshComponent* MainMesh = Cast<USkeletalMeshComponent>(OtherComp);
				if(MainMesh){
					MainChar->MainPlayerController->RemoveEnemyHealthBar();
				}
			}

			GetWorldTimerManager().ClearTimer(AttackTimer); // if Character escaped, no need to delay the Enemy attack anymore
		}
	}


}

void AEnemy::MoveToTarget(AMainCharacter* Target) {

	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

	if(AIController) {
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(10.0f);

		FNavPathSharedPtr NavPath;

		AIController->MoveTo(MoveRequest, &NavPath);

		// Show the path points that AI generates
		// TArray<FNavPathPoint> PathPoints = NavPath->GetPathPoints();
		// for(auto Point : PathPoints) {
		// 	FVector Location = Point.Location;
		// 	UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.0f, 12, FLinearColor::Red, 10.0, 2.0);

		// }

	}
}							

void AEnemy::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
                            int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult){

    UE_LOG(LogTemp, Warning, TEXT("ENEMY: CombatOnOverlapBegin"));

    if (OtherActor && Alive()) {
        AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
        if(MainChar && MainChar->HitParticles) {
            const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName("TipSocket");
            if(TipSocket) {
                UE_LOG(LogTemp, Warning, TEXT("ENEMY: TipSocket"));
                FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
                UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MainChar->HitParticles, SocketLocation, FRotator(0.0f), false);
            }
            
        }
		// TODO: BUGGY, RANDOMLY CRASHES UE4
        // if(MainChar->HitSoundCue) {
        //     UGameplayStatics::PlaySound2D(this, MainChar->HitSoundCue);
        // }
		if(DamageTypeClass) {
			// causes the TakeDamage function to be called in the damage taker (MainCHaracter)
			UGameplayStatics::ApplyDamage(MainChar, Damage, AIController, this, DamageTypeClass);
		}
    }
}



void AEnemy::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {

}	


void AEnemy::ActivateCollision() {
	if(SwingSoundCue){
		UGameplayStatics::PlaySound2D(this, SwingSoundCue);
	}	
    CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AEnemy::DeactivateCollision() {
    CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemy::AttackEnd() {
	bAttacking = false;

	if(bOverlappingCombatSphere) {

		float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
	}
}

void AEnemy::Attack() {
	if(Alive() && bHasValidTarget) {
		if (AIController) {
			AIController->StopMovement();
			SetEnemyMovementStatus((EEnemyMovementStatus::EMS_Attacking));
			if(!bAttacking) {
				bAttacking = true;
				UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
				if(AnimInstance){
					AnimInstance->Montage_Play(CombatMontage, 1.35f);
					AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
				}
			}
		}
	}

}

float AEnemy::TakeDamage(float DamageAmount,struct FDamageEvent const & DamageEvent, 
					class AController * EventInstigator, AActor * DamageCauser) {
	if(Health - DamageAmount <= 0.0f) {
		Health -= DamageAmount;
		Die(DamageCauser);
	}
	else {
		Health -= DamageAmount;
	}
	return DamageAmount;
}

void AEnemy::Die(AActor* Causer) {
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);

	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance) {
		AnimInstance->Montage_Play(CombatMontage, 1.0f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bAttacking = false;

	AMainCharacter* MainChar = Cast<AMainCharacter>(Causer);
	if(MainChar){
		MainChar->UpdateCombatTarget();
	}
}

void AEnemy::DeathEnd() {
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;

	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemy::Disappear, DeathDelay);
}

bool AEnemy::Alive() {
	return GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead;
}

void AEnemy::Disappear() {
	Destroy();
}

