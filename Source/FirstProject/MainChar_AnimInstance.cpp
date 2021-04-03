// Fill out your copyright notice in the Description page of Project Settings.


#include "MainChar_AnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MainCharacter.h"

void UMainChar_AnimInstance::NativeInitializeAnimation() {
    if (Pawn == nullptr) {
        Pawn = TryGetPawnOwner();
        if (Pawn) {
            MainChar = Cast<AMainCharacter>(Pawn);
        }
    }
}

void UMainChar_AnimInstance::UpdateAnimationProperties() {
    if (Pawn == nullptr) {
        Pawn = TryGetPawnOwner();
    }

    if (Pawn) {
        FVector Speed = Pawn->GetVelocity();
        FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
        MovementSpeed = LateralSpeed.Size();

        bIsInAir = Pawn->GetMovementComponent()->IsFalling();

        if(MainChar == nullptr) {
            MainChar = Cast<AMainCharacter>(Pawn);
        }
    }
}


