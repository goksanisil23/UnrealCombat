// Fill out your copyright notice in the Description page of Project Settings.


#include "ColliderMovementComponent.h"

void UColliderMovementComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) {
    
    // Super calls the parent version
    // think of super as:
    // using Super = Base;
    // by using Super, we can extend the functionality of base class
    // without Super, we are completely overwriting the function
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // make sure all necessary pieces exist
    if (!PawnOwner || !UpdatedComponent || ShouldSkipUpdate(DeltaTime)) {
        return;
    }

    // returns the InputVector and then clears it out (also normalizing)
    // this InputVector comes from the MoveForward, MoveRight functions which used AddInputVector 
    FVector DesiredMovementThisFrame = ConsumeInputVector().GetClampedToMaxSize(1.0f);

    if(!DesiredMovementThisFrame.IsNearlyZero()) {
        FHitResult Hit;
        // moving the actual pawn
        SafeMoveUpdatedComponent(DesiredMovementThisFrame, UpdatedComponent->GetComponentRotation(), true, Hit);

        // if we bump into something, slide along the side of it
        if(Hit.IsValidBlockingHit()) {
            SlideAlongSurface(DesiredMovementThisFrame, 1.f - Hit.Time, Hit.Normal, Hit);

            UE_LOG(LogTemp, Warning, TEXT("%f %f %f %f Valid Blocking Hit"), 
                                        DeltaTime, DesiredMovementThisFrame.X, DesiredMovementThisFrame.Y, DesiredMovementThisFrame.Z);
        }
    }

}



