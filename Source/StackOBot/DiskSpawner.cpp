#include "DiskSpawner.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// ©¤©¤©¤ Constructor ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
ADiskSpawner::ADiskSpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

// ©¤©¤©¤ BeginPlay ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
void ADiskSpawner::BeginPlay()
{
    Super::BeginPlay();

    CachedPlayer = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    SpawnAllDisks();
}

// ©¤©¤©¤ Tick ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
void ADiskSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    CheckGroundHits();
    CheckRedPromotion();
    CheckGreenUnfreeze();
}

// ©¤©¤©¤ Public ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤

/**
 * Called by UShootingComponent when the player's laser hits a falling disk.
 * Applies the red/green freeze rule based on whether the disk is above or below
 * the player's center.
 */
void ADiskSpawner::NotifyDiskHit(AFallingDisk* HitDisk, ACharacter* Player)
{
    if (!HitDisk || !Player) return;
    if (HitDisk->DiskState != EDiskState::Falling) return;  // Already frozen

    const float PlayerZ = Player->GetActorLocation().Z;
    const float DiskZ = HitDisk->GetActorLocation().Z;

    if (DiskZ > PlayerZ)
    {
        // Disk is above the player ¡ú freeze Red.
        // Unfreeze the previous red disk first (restores its stored fall speed).
        if (RedDisk && RedDisk != HitDisk)
        {
            RedDisk->Unfreeze();
        }
        HitDisk->FreezeRed();
        RedDisk = HitDisk;
    }
    else
    {
        // Disk is at or below the player ¡ú freeze Green immediately.
        HitDisk->FreezeGreen();
    }
}

// ©¤©¤©¤ Private ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
void ADiskSpawner::SpawnAllDisks()
{
    if (!DiskClass)
    {
        UE_LOG(LogTemp, Warning, TEXT("ADiskSpawner: DiskClass is not set!"));
        return;
    }

    const int32 Total = GridColumns * GridRows;
    Disks.SetNum(Total);

    FActorSpawnParameters Params;
    Params.Owner = this;

    for (int32 Row = 0; Row < GridRows; ++Row)
    {
        for (int32 Col = 0; Col < GridColumns; ++Col)
        {
            const int32 Index = Col + Row * GridColumns;
            FVector SpawnLoc = CellSpawnLocation(Col, Row);

            AFallingDisk* Disk = GetWorld()->SpawnActor<AFallingDisk>(
                DiskClass, SpawnLoc, FRotator::ZeroRotator, Params);

            if (Disk)
            {
                Disk->Initialize(RandomSpeed());
                Disks[Index] = Disk;
            }
        }
    }
}

FVector ADiskSpawner::CellSpawnLocation(int32 Col, int32 Row) const
{
    // Center the grid on the spawner actor's XY position.
    const float GridWidth = GridColumns * CellSize;
    const float GridDepth = GridRows * CellSize;

    const float OffsetX = (Col + 0.5f) * CellSize - GridWidth * 0.5f;
    const float OffsetY = (Row + 0.5f) * CellSize - GridDepth * 0.5f;

    FVector Origin = GetActorLocation();
    return FVector(Origin.X + OffsetX,
        Origin.Y + OffsetY,
        Origin.Z + SpawnHeightOffset);
}

float ADiskSpawner::RandomSpeed() const
{
    return FMath::RandRange(MinSpeed, MaxSpeed);
}

/** Respawn any falling disk that has dropped below GroundZ. */
void ADiskSpawner::CheckGroundHits()
{
    for (int32 i = 0; i < Disks.Num(); ++i)
    {
        AFallingDisk* Disk = Disks[i];
        if (!Disk) continue;
        if (Disk->DiskState != EDiskState::Falling) continue;

        if (Disk->GetActorLocation().Z < GroundZ)
        {
            const int32 Col = i % GridColumns;
            const int32 Row = i / GridColumns;
            Disk->Respawn(CellSpawnLocation(Col, Row), RandomSpeed());
        }
    }
}

/**
 * If the player has climbed above the current Red disk, promote it to Green.
 * This opens the slot for the player to freeze a new disk above them.
 */
void ADiskSpawner::CheckRedPromotion()
{
    if (!RedDisk || !CachedPlayer) return;
    if (RedDisk->DiskState != EDiskState::FrozenRed) return;

    const float PlayerZ = CachedPlayer->GetActorLocation().Z;
    const float DiskZ = RedDisk->GetActorLocation().Z;

    if (PlayerZ > DiskZ)   // Player center is now above the disk center
    {
        RedDisk->PromoteToGreen();
        RedDisk = nullptr;
    }
}

/**
 * Unfreeze any Green disk the player has fallen below.
 * This creates gameplay pressure: falling back down costs you your platforms.
 */
void ADiskSpawner::CheckGreenUnfreeze()
{
    if (!CachedPlayer) return;

    const float PlayerZ = CachedPlayer->GetActorLocation().Z;

    for (AFallingDisk* Disk : Disks)
    {
        if (!Disk) continue;
        if (Disk->DiskState != EDiskState::FrozenGreen) continue;

        if (PlayerZ < Disk->GetActorLocation().Z)  // Player fell below this disk
        {
            Disk->Unfreeze();
        }
    }
}