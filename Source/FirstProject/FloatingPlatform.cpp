// Fill out your copyright notice in the Description page of Project Settings.


#include "FloatingPlatform.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"

// Sets default values
AFloatingPlatform::AFloatingPlatform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;

	StartPoint = FVector(0.f);
	EndPoint = FVector(0.f);
	InterpSpeed = 4.0f;
	InterpTime = 1.0f;
	bInterping = false;

}

// Called when the game starts or when spawned
void AFloatingPlatform::BeginPlay()
{
	Super::BeginPlay();

	StartPoint = GetActorLocation();
	EndPoint += StartPoint; // Since the EndPoint coordinates are local w.r.t platform (due to MakeWidgetEdit property)

	bInterping = false;

	// delays the platform movement at the beginning
	GetWorldTimerManager().SetTimer(InterpTimer, this, &AFloatingPlatform::ToggleInterping, InterpTime);

	Distance = (EndPoint - StartPoint).Size();
	
}

// Called every frame
void AFloatingPlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(bInterping) {
		FVector CurrentLocation = GetActorLocation();
		FVector Interp = FMath::VInterpTo(CurrentLocation, EndPoint, DeltaTime, InterpSpeed);
		SetActorLocation(Interp);

		float DistanceTraveled = (CurrentLocation - StartPoint).Size();
		if((Distance - DistanceTraveled) <= 1.f) {
			ToggleInterping(); // stop the platform movement when distance is small enough
			GetWorldTimerManager().SetTimer(InterpTimer, this, &AFloatingPlatform::ToggleInterping, InterpTime); // delay before going back
			SwapVectors(StartPoint, EndPoint);
		}
	}

}

void AFloatingPlatform::SwapVectors(FVector& Vec1, FVector& Vec2) {
	FVector temp = Vec1;
	Vec1 = Vec2;
	Vec2 = temp;
}

void AFloatingPlatform::ToggleInterping(){
	bInterping = !bInterping;
}
