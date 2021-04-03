// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Collider.generated.h"

// Forward declaring classes that are used for creating pointers (we dont need access to their info within header)
class UStaticMeshComponent;
class USphereComponent;
class UCameraComponent;
class USpringArmComponent;
class UColliderMovementComponent;

UCLASS()
class FIRSTPROJECT_API ACollider : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ACollider();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UStaticMeshComponent* MeshComponent;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USphereComponent* SphereComponent;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere, Category = "Mesh")
	USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleAnywhere, Category = "Movement")
	UColliderMovementComponent* MyMovementComponent;

	// We can override since UColliderMovementComponent derives from UPawnMovementComponent
	virtual UPawnMovementComponent* GetMovementComponent() const override;

	FORCEINLINE UStaticMeshComponent* GetMeshComponent() {return MeshComponent;}
	FORCEINLINE void SetMeshComponent(UStaticMeshComponent* Mesh) {MeshComponent = Mesh;}
	
	FORCEINLINE USphereComponent* GetSphereComponent() {return SphereComponent;}
	FORCEINLINE void SetSphereComponent(USphereComponent* Sphere) {SphereComponent = Sphere;}	

	FORCEINLINE UCameraComponent* GetCameraComponent() {return CameraComponent;}
	FORCEINLINE void SetCameraComponent(UCameraComponent* Camera) {CameraComponent = Camera;}
	
	FORCEINLINE USpringArmComponent* GetSpringArmComponent() {return SpringArmComponent;}
	FORCEINLINE void SetSpringArmComponent(USpringArmComponent* SpringArm) {SpringArmComponent = SpringArm;}		

private:

	void MoveForward(float input);
	void MoveRight(float input);

	void PitchCamera(float AxisValue);
	void YawCamera(float AxisValue);
	
	FVector2D CameraInput;

};
