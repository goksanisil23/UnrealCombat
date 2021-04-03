// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"

#include "MainCharacter.h"
#include "Enemy.h"

#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"

AExplosive::AExplosive() {
    Damage = 15.0f;
}


void AExplosive::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
								int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) {

    // Calls the functionality in the parent
    Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep,SweepResult);
    
    // Retrieve the actor that overlaps and check if it's the main character
    if (OtherActor) {
        AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
        AEnemy* Enemy = Cast<AEnemy>(OtherActor);

        if (MainChar || Enemy) {
            // First spawn an emitter on overlap and then destroy everything
            if (OverlapParticles) {
                UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticles, GetActorLocation(), FRotator(0), true);
            }
            if (OverlapSound) {
                UGameplayStatics::PlaySound2D(this, OverlapSound);
            }            
            
            UGameplayStatics::ApplyDamage(OtherActor, Damage, nullptr, this, DamageTypeClass);

            Destroy(); // Garbage collection will destroy all members tagged with UPROPERTY
        }
    }          

}
	
void AExplosive::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
								UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
                                    
    // Calls the functionality in the parent
    Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}

