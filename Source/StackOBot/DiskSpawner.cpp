#include "DiskSpawner.h"
#include "PlayerHUDWidget.h"

#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
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
    HighestZ = CachedPlayer ? CachedPlayer->GetActorLocation().Z : 0.f;

    SpawnAllDisks();

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        // Create HUD and show it immediately
        if (HUDWidgetClass)
        {
            HUDWidget = CreateWidget<UPlayerHUDWidget>(PC, HUDWidgetClass);
            if (HUDWidget) HUDWidget->AddToViewport();
        }

        // Pre-create the win screen but don't show it yet
        if (WinScreenClass)
        {
            WinScreenWidget = CreateWidget<UUserWidget>(PC, WinScreenClass);
        }
    }
}

// ©¤©¤©¤ Tick ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
void ADiskSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!CachedPlayer) return;

    const float PlayerZ = CachedPlayer->GetActorLocation().Z;
    if (PlayerZ > HighestZ) HighestZ = PlayerZ;

    CheckGroundHits();
    CheckRedPromotion();
    CheckGreenUnfreeze();
    CheckWinCondition(PlayerZ);
    UpdateHUD(PlayerZ);
}

// ©¤©¤©¤ Public ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤

void ADiskSpawner::NotifyDiskHit(AFallingDisk* HitDisk, ACharacter* Player)
{
    if (!HitDisk || !Player) return;
    if (HitDisk->DiskState != EDiskState::Falling) return;

    const float PlayerZ = Player->GetActorLocation().Z;
    const float DiskZ = HitDisk->GetActorLocation().Z;

    if (DiskZ > PlayerZ)
    {
        if (RedDisk && RedDisk != HitDisk) RedDisk->Unfreeze();
        HitDisk->FreezeRed();
        RedDisk = HitDisk;
    }
    else
    {
        HitDisk->FreezeGreen();
    }
}

/** Called by UI_WinScreen's "Play Infinite Mode" button via Blueprint. */
void ADiskSpawner::StartInfiniteMode()
{
    bInfiniteMode = true;
    bGameWon = false;

    if (WinScreenWidget && WinScreenWidget->IsInViewport())
        WinScreenWidget->RemoveFromParent();

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        PC->SetInputMode(FInputModeGameOnly());
        PC->SetShowMouseCursor(false);
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
            AFallingDisk* Disk = GetWorld()->SpawnActor<AFallingDisk>(
                DiskClass, CellSpawnLocation(Col, Row), FRotator::ZeroRotator, Params);
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
    const float GridWidth = GridColumns * CellSize;
    const float GridDepth = GridRows * CellSize;

    const float OffsetX = (Col + 0.5f) * CellSize - GridWidth * 0.5f;
    const float OffsetY = (Row + 0.5f) * CellSize - GridDepth * 0.5f;

    const FVector Origin = GetActorLocation();

    // Spawn above the player's current height, not the spawner's fixed height.
    const float BaseZ = CachedPlayer
        ? CachedPlayer->GetActorLocation().Z + SpawnHeightOffset
        : Origin.Z + SpawnHeightOffset;

    return FVector(Origin.X + OffsetX, Origin.Y + OffsetY, BaseZ);
}

float ADiskSpawner::RandomSpeed() const
{
    return FMath::RandRange(MinSpeed, MaxSpeed);
}

void ADiskSpawner::CheckGroundHits()
{
    const float PlayerZ = CachedPlayer ? CachedPlayer->GetActorLocation().Z : 0.f;

    for (int32 i = 0; i < Disks.Num(); ++i)
    {
        AFallingDisk* Disk = Disks[i];
        if (!Disk || Disk->DiskState != EDiskState::Falling) continue;

        const float DiskZ = Disk->GetActorLocation().Z;

        // Respawn if the disk hit the floor OR drifted too far below the player.
        if (DiskZ < GroundZ || DiskZ < (PlayerZ - RespawnBelowOffset))
        {
            const int32 Col = i % GridColumns;
            const int32 Row = i / GridColumns;
            Disk->Respawn(CellSpawnLocation(Col, Row), RandomSpeed());
        }
    }
}

void ADiskSpawner::CheckRedPromotion()
{
    if (!RedDisk || !CachedPlayer) return;
    if (RedDisk->DiskState != EDiskState::FrozenRed) return;

    if (CachedPlayer->GetActorLocation().Z > RedDisk->GetActorLocation().Z)
    {
        RedDisk->PromoteToGreen();
        RedDisk = nullptr;
    }
}

void ADiskSpawner::CheckGreenUnfreeze()
{
    if (!CachedPlayer) return;
    const float PlayerZ = CachedPlayer->GetActorLocation().Z;

    for (AFallingDisk* Disk : Disks)
    {
        if (!Disk || Disk->DiskState != EDiskState::FrozenGreen) continue;
        if (PlayerZ < Disk->GetActorLocation().Z)
            Disk->Unfreeze();
    }
}

void ADiskSpawner::CheckWinCondition(float PlayerZ)
{
    if (bGameWon || bInfiniteMode) return;
    if (PlayerZ >= WinZ)
    {
        bGameWon = true;
        ShowWinScreen();
    }
}

void ADiskSpawner::UpdateHUD(float PlayerZ)
{
    if (HUDWidget)
        HUDWidget->UpdateValues(PlayerZ, HighestZ);
}

void ADiskSpawner::ShowWinScreen()
{
    if (!WinScreenWidget) return;
    WinScreenWidget->AddToViewport(10);   // zOrder 10 so it renders on top

    APlayerController* PC = UGameplayStatics::GetPlayerController(this, 0);
    if (PC)
    {
        PC->SetInputMode(FInputModeUIOnly());
        PC->SetShowMouseCursor(true);
    }
}