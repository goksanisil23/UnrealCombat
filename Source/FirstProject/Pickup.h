// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Item.h"
#include "Pickup.generated.h"

/**
 * 
 */
UCLASS()
class FIRSTPROJECT_API APickup : public AItem
{
	GENERATED_BODY()
	
public:
	APickup();


	// UFUNCTION() // overridden functions do not need the UFUNCTION again, since the parent has it
	virtual void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
								int32 OtherBodyIndex, bool bFromSweep, const FHitResult & SweepResult) override;
	
	// UFUNCTION()
	virtual void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
								UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) override;	
	
	
	UFUNCTION(BlueprintImplementableEvent, Category = "Pickup")
	void OnPickupBP(class AMainCharacter* Target);

};
