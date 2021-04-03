// Fill out your copyright notice in the Description page of Project Settings.


#include "Collider.h"

// including forward declared classes in the .cpp since we need to access their data
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "ColliderMovementComponent.h"

#include "UObject/ConstructorHelpers.h"


// Sets default values
ACollider::ACollider()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	// Since the collider boundary is defined by sphere, we make sphere the root component
	SetRootComponent(SphereComponent); 
	SphereComponent->InitSphereRadius(40.f);
	SphereComponent->SetCollisionProfileName(TEXT("Pawn"));

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(GetRootComponent());

	// Assigning a specific mesh programmatically to the MeshComponent
	static ConstructorHelpers::FObjectFinder<UStaticMesh> 
		MeshComponentAsset(TEXT("StaticMesh'/Game/StarterContent/Shapes/Shape_Sphere.Shape_Sphere'"));

	if(MeshComponentAsset.Succeeded()) {
		MeshComponent->SetStaticMesh(MeshComponentAsset.Object);
		MeshComponent->SetRelativeLocation(FVector(0.f, 0.f, -40.f));
		MeshComponent->SetWorldScale3D(FVector(0.8f, 0.8f, 0.8f));
	}

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->SetRelativeRotation(FRotator(-45.f, 0.f, 0.f));
	SpringArmComponent->TargetArmLength = 400.f;
	SpringArmComponent->bEnableCameraLag = true;
	SpringArmComponent->CameraLagSpeed = 3.0f;

	// Creating camera
	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	// CameraComponent->SetRelativeLocation();
	// CameraComponent->SetRelativeRotation();

	MyMovementComponent = CreateDefaultSubobject<UColliderMovementComponent>(TEXT("MyMovementComponent"));
	MyMovementComponent->UpdatedComponent = RootComponent; // The component we move and update.

	CameraInput = FVector2D(0.f, 0.f); 

	// possessing --> playing through this Actor's camera
	AutoPossessPlayer = EAutoReceiveInput::Player0;

}

// Called when the game starts or when spawned
void ACollider::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ACollider::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Rotating the actor based on the mouse movement --> camera moves as well since its attached to SpringArm which is attached to root
	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += CameraInput.X;
	SetActorRotation(NewRotation);

	// Moving the spring arm for the camera pitch
	FRotator NewSpringArmRotation = SpringArmComponent->GetComponentRotation();
	NewSpringArmRotation.Pitch = FMath::Clamp(NewSpringArmRotation.Pitch+CameraInput.Y, -80.f, -15.f);
	SpringArmComponent->SetWorldRotation(NewSpringArmRotation);

}

// Called to bind functionality to input
void ACollider::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &ACollider::MoveForward);
	PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &ACollider::MoveRight);

	PlayerInputComponent->BindAxis(TEXT("CameraPitch"), this, &ACollider::PitchCamera);
	PlayerInputComponent->BindAxis(TEXT("CameraYaw"), this, &ACollider::YawCamera);	

}

// input = [-1,1]
void ACollider::MoveForward(float input) {

	FVector Forward = GetActorForwardVector();
	if (MyMovementComponent) {
		MyMovementComponent->AddInputVector(Forward * input);
	}
}

// input = [-1,1]
void ACollider::MoveRight(float input) {

	FVector Right = GetActorRightVector();
	if (MyMovementComponent) {
		MyMovementComponent->AddInputVector(Right * input);
	}
}

UPawnMovementComponent* ACollider::GetMovementComponent() const {
	return MyMovementComponent;
}

void ACollider::PitchCamera(float AxisValue) {
	CameraInput.Y = AxisValue;
}

void ACollider::YawCamera(float AxisValue) {
	CameraInput.X = AxisValue;
}

