#include "FallingDisk.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"

// ©¤©¤©¤ Constructor ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
AFallingDisk::AFallingDisk()
{
    PrimaryActorTick.bCanEverTick = true;

    DiskMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("DiskMesh"));
    RootComponent = DiskMesh;

    // Block everything so the player can stand on frozen disks.
    // SetNotifyRigidBodyCollision enables the OnComponentHit callback.
    DiskMesh->SetCollisionProfileName(TEXT("BlockAll"));
    DiskMesh->SetNotifyRigidBodyCollision(true);
}

void AFallingDisk::BeginPlay()
{
    Super::BeginPlay();
    DiskMesh->OnComponentHit.AddDynamic(this, &AFallingDisk::OnDiskHit);
}

// ©¤©¤©¤ Tick ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
void AFallingDisk::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Only move when falling; frozen disks are stationary.
    if (DiskState == EDiskState::Falling)
    {
        AddActorWorldOffset(FVector(0.f, 0.f, -FallSpeed * DeltaTime));
    }
}

// ©¤©¤©¤ API ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
void AFallingDisk::Initialize(float Speed)
{
    FallSpeed = Speed;
    StoredFallSpeed = Speed;
    DiskState = EDiskState::Falling;
    ApplyMaterial(Mat_Falling);
}

void AFallingDisk::FreezeRed()
{
    StoredFallSpeed = FallSpeed;
    DiskState = EDiskState::FrozenRed;
    ApplyMaterial(Mat_FrozenRed);
}

void AFallingDisk::FreezeGreen()
{
    StoredFallSpeed = FallSpeed;
    DiskState = EDiskState::FrozenGreen;
    ApplyMaterial(Mat_FrozenGreen);
}

void AFallingDisk::PromoteToGreen()
{
    // StoredFallSpeed is already set from when it was frozen red.
    DiskState = EDiskState::FrozenGreen;
    ApplyMaterial(Mat_FrozenGreen);
}

void AFallingDisk::Unfreeze()
{
    FallSpeed = StoredFallSpeed;
    DiskState = EDiskState::Falling;
    ApplyMaterial(Mat_Falling);
}

void AFallingDisk::Respawn(FVector NewLocation, float NewSpeed)
{
    SetActorLocation(NewLocation);
    Initialize(NewSpeed);
}

// ©¤©¤©¤ Private ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
void AFallingDisk::ApplyMaterial(UMaterialInterface* Mat)
{
    if (Mat && DiskMesh)
    {
        DiskMesh->SetMaterial(0, Mat);
    }
}

void AFallingDisk::OnDiskHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse,
    const FHitResult& Hit)
{
    // Only falling disks knock the player back.
    if (DiskState != EDiskState::Falling) return;

    ACharacter* HitChar = Cast<ACharacter>(OtherActor);
    if (!HitChar) return;

    UCharacterMovementComponent* Move = HitChar->GetCharacterMovement();
    if (Move && !Move->IsMovingOnGround())
    {
        // Push the player downward and slightly away from the disk center.
        FVector Dir = (HitChar->GetActorLocation() - GetActorLocation()).GetSafeNormal();
        Dir.Z = -0.6f;
        HitChar->LaunchCharacter(Dir.GetSafeNormal() * KnockbackForce,
            /*bXYOverride=*/true, /*bZOverride=*/true);
    }
}