// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnVolume.generated.h"

// Forward declaring some classes
class UBoxComponent;
class ACritter;

UCLASS()
class FIRSTPROJECT_API ASpawnVolume : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASpawnVolume();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Spawning")
	UBoxComponent* SpawningBox;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Spawning")
	TSubclassOf<ACritter> PawnToSpawn;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Pure function: - Cannot modify state/members of its class
	// 				  - Connected to data pins and automatically executed by compiler when their data is needed
	// 				  - (Does not have input pins for line of execution in EventGraph)
	// 			      - Created by "BlueprintPure" keyword in UFUNCTION
	// Gets a random spawn point and returns that
	UFUNCTION(BlueprintPure, Category = "Spawning") // Like a getter/accessor
	FVector GetSpawnPoint();

	// BlueprintCallable makes is explicitly executed via event wires in node graph
	// BlueprintNativeEvent add hybrid functionality so some parts implemented in c++, and some parts via events in blueprint
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Spawning")
	void SpawnOurPawn(UClass* ToSpawn, const FVector& Location);

};
