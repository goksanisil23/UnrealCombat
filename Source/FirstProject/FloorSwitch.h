// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FloorSwitch.generated.h"

// Forward declaring some classes
class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class FIRSTPROJECT_API AFloorSwitch : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFloorSwitch();

	// Overlap volume for trigger functionality
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Floor Switch")
	UBoxComponent* TriggerBox;

	// Switch for the character to step on
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Floor Switch")
	UStaticMeshComponent* FloorSwitch;

	// Door to move when the floor switch is stepped on
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Floor Switch")
	UStaticMeshComponent* Door;

	// Initial location for the door
	UPROPERTY(BlueprintReadWrite, Category = "Floor Switch")
	FVector InitialDoorLocation;

	// Initial location for the floor switch
	UPROPERTY(BlueprintReadWrite, Category = "Floor Switch")
	FVector InitialFloorSwitchLocation;

	FTimerHandle SwitchTimerHandle;

	// Time to close the door after stepped off of trigger
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Floor Switch")
	float SwitchTime;

	// If the character is still on the door switch
	bool bCharacterOnSwitch;

	void CloseDoor();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION()
	void OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, 
						int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);								  

	UFUNCTION(BlueprintImplementableEvent, Category = "Floor Switch") // We dont need to give implementation in C++, will be called in Blueprint
	void RaiseDoor();

	UFUNCTION(BlueprintImplementableEvent, Category = "Floor Switch") // We dont need to give implementation in C++, will be called in Blueprint
	void LowerDoor();	

	UFUNCTION(BlueprintImplementableEvent, Category = "Floor Switch") // We dont need to give implementation in C++, will be called in Blueprint
	void RaiseFloorSwitch();

	UFUNCTION(BlueprintImplementableEvent, Category = "Floor Switch") // We dont need to give implementation in C++, will be called in Blueprint
	void LowerFloorSwitch();

	// Impure function: - Free to modify the state/members of the class
	//                  - Must be explicitly executed by connecting exec wires in EventGraph
	// 					- (Hasinput pins for line of execution in EventGraph)
	// 					- Created by "BlueprintCallable" keyword in UFUNCTION
	UFUNCTION(BlueprintCallable, Category = "Floor Switch") // Can only call from blueprint, but need to implement in C++
	void UpdateDoorLocation(float Z);

	UFUNCTION(BlueprintCallable, Category = "Floor Switch") // Can only call from blueprint, but need to implement in C++
	void UpdateFloorSwitchLocation(float Z);
	
};
