// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"

#include "MainCharacter.h"

#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"

APickup::APickup() {
}


void APickup::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
								int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) {

    // Calls the functionality in the parent
    Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep,SweepResult);

    // Retrieve the actor that overlaps and check if it's the main character
    if (OtherActor) {
        AMainCharacter* MainChar = Cast<AMainCharacter>(OtherActor);
        if (MainChar) {
            OnPickupBP(MainChar);
            // First spawn an emitter on overlap and then destroy everything
            if (OverlapParticles) {
                UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverlapParticles, GetActorLocation(), FRotator(0), true);
            }
            if (OverlapSound) {
                UGameplayStatics::PlaySound2D(this, OverlapSound);
            }   

            MainChar->PickupLocations.Add(GetActorLocation());

            Destroy(); // Garbage collection will destroy all members tagged with UPROPERTY
        }
    }                  

}
	
void APickup::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
								UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) {
                                    
    // Calls the functionality in the parent
    Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
    UE_LOG(LogTemp, Warning, TEXT("Pickup:: OnOverlapEnd"))
}


