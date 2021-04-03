// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "MainCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "Enemy.h"
#include "Engine/SkeletalMeshSocket.h"

AWeapon::AWeapon() {
    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMesh"));
    SkeletalMesh->SetupAttachment(GetRootComponent());

    CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
    CombatCollision->SetupAttachment(GetRootComponent());

    bWeaponParticle = false;

    WeaponState = EWeaponState::EWS_Pickup;

    Damage = 25.0f;

}

void AWeapon::BeginPlay() {
    Super::BeginPlay();

    CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision); // only when animation is in "hitting" position, weapon will collide
    CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::CombatOnOverlapBegin);
    CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AWeapon::CombatOnOverlapEnd);
}

void AWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
								int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) {

    // Calls the functionality in the parent
    Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep,SweepResult);

    if ( (WeaponState == EWeaponState::EWS_Pickup) && OtherActor) {
        AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
        if (MainChar) {
            // WE dont immediately equip the weapon on collision, player will choose to pick it up or not
            MainChar->SetActiveOverlappingItem(this);
        }
    }

}
	
void AWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
								UPrimitiveComponent* OtherComp, int32 OtherBodyIndex){

    // Calls the functionality in the parent
    Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

    if (OtherActor) {
        AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
        if (MainChar) {
            MainChar->SetActiveOverlappingItem(nullptr);
        }
    }

}		

// Called at LMB event if ActiveOverlappingItem exists
void AWeapon::Equip(AMainCharacter* MainChar){
    if(MainChar) {

        SetInstigator(MainChar->GetController());

        // DOnt zoom the camera if the sword is in the way
        SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
        SkeletalMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

        SkeletalMesh->SetSimulatePhysics(false);

        const USkeletalMeshSocket* RightHandSocket = MainChar->GetMesh()->GetSocketByName("RightHandSocket");
        if(RightHandSocket) {
            RightHandSocket->AttachActor(this, MainChar->GetMesh());
            bRotate = false;

            MainChar->SetEquippedWeapon(this);
            MainChar->SetActiveOverlappingItem(nullptr);

            // play sound on equipping
            if(OnEquipSoundCue) {
                UGameplayStatics::PlaySound2D(this, OnEquipSoundCue);
            }
            if(!bWeaponParticle) { // if user dont want to keep the particles on picking up
                IdleParticlesComponent->Deactivate();
            }
        }
    }
}


void AWeapon::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
                            int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult){
    
    if (OtherActor) {
        AEnemy* Enemy = Cast<AEnemy>(OtherActor);
        if(Enemy && Enemy->HitParticles) {
            const USkeletalMeshSocket* WeaponSocket = SkeletalMesh->GetSocketByName("WeaponSocket");
            if(WeaponSocket) {
                UE_LOG(LogTemp, Warning, TEXT("Weapon: CombatOnOverlapBegin"));
                FVector SocketLocation = WeaponSocket->GetSocketLocation(SkeletalMesh);
                UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Enemy->HitParticles, SocketLocation, FRotator(0.0f), false);
            }
            
        }
        // TODO: Sound cue is crashing randomly
        // if(Enemy->HitSoundCue) {
        //     UGameplayStatics::PlaySound2D(this, Enemy->HitSoundCue);
        // }
        if(DamageTypeClass) {
            UGameplayStatics::ApplyDamage(Enemy, Damage, WeaponInstigator, this, DamageTypeClass);
        }

    }
}



void AWeapon::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                            UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {

}	


void AWeapon::ActivateCollision() {
    CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}

void AWeapon::DeactivateCollision() {
    CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}