# Stack O Bot – C++ Implementation Guide

## What We Are Building

The new mechanic adds falling disks the player must shoot and freeze to use as
stepping-stone platforms for climbing to a high-altitude win zone.

| Feature | Description |
|---|---|
| Falling Disks | Spawn in a grid above the level; fall at random speeds |
| Shoot to Freeze | RMB aims (camera shifts + crosshair); LMB fires a hitscan laser |
| Red Disk | One disk above the player can be frozen red; shooting another above unfreezes the old one |
| Green Disk | Any disk at or below the player is frozen green; it unfreezes the moment the player falls below it |
| Red → Green | A red disk automatically becomes green once the player climbs above it |
| Knockback | A falling disk that hits the player while airborne launches them downward |
| Win Zone | A trigger volume at high altitude ends the level |

---

## Architecture at a Glance

```
ADiskSpawner          – placed once in the level; owns the disk grid
    └── TArray<AFallingDisk*>   – one per grid cell

AFallingDisk          – moves downward each tick; responds to freeze/unfreeze calls
    └── EDiskState    – Falling | FrozenRed | FrozenGreen

UShootingComponent    – UActorComponent you add to BP_Bot in the Blueprint editor
    ├── ADS camera shift (lerps Spring Arm socket offset)
    ├── Line-trace laser on Fire
    └── calls ADiskSpawner::NotifyDiskHit(disk, player)

Input wiring          – done inside BP_Bot's Event Graph (Blueprint side)
    RMB Pressed/Released  → ShootingComponent::StartADS / StopADS
    LMB Pressed           → ShootingComponent::Fire

Win Zone              – a Blueprint Trigger Volume placed in the level (no C++ needed)
```

---

## Phase 0 – Convert the Project to C++

> **Before you start:** Make sure **Visual Studio 2022** is installed with the
> **"Game development with C++"** workload and the **Unreal Engine** optional
> component checked inside that workload.

1. In the Unreal Editor menu bar choose **Tools → New C++ Class**.
2. Select **Actor** as the parent class. Name it `FallingDisk`. Click
   **Create Class**.
3. Unreal will ask to compile — click **Yes**. The editor will restart.
4. After restart your project root now has:
   - `Source/StackOBot/StackOBot.Build.cs`
   - `Source/StackOBot/FallingDisk.h` and `FallingDisk.cpp`
   - `StackOBot.sln` in the project root
5. Open `StackOBot.sln` in Visual Studio 2022. You will do all C++ editing there.

> **Tip:** Every time you add a new `.h`/`.cpp` file manually, right-click
> `StackOBot.uproject` in Windows Explorer and choose
> **Generate Visual Studio project files** so the .sln picks up the new files.

---

## Phase 1 – Build Configuration

Open `Source/StackOBot/StackOBot.Build.cs` and replace everything with:

```csharp
using UnrealBuildTool;

public class StackOBot : ModuleRules
{
    public StackOBot(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",    // Enhanced Input system (already a plugin in this project)
            "Niagara",          // VFX for laser beam (already a plugin)
            "UMG",              // Crosshair widget
        });
    }
}
```

Save the file, then **Build → Build Solution** in Visual Studio before
continuing. A successful build here confirms the module wiring is correct.

---

## Phase 2 – FallingDisk Actor

This actor represents one disk. It moves downward each tick when falling, changes
color to show its freeze state, and knocks back an airborne player on impact.

### `Source/StackOBot/FallingDisk.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FallingDisk.generated.h"

// ─── Disk state ────────────────────────────────────────────────────────────────
UENUM(BlueprintType)
enum class EDiskState : uint8
{
    Falling      UMETA(DisplayName = "Falling"),
    FrozenRed    UMETA(DisplayName = "Frozen Red  (one above player)"),
    FrozenGreen  UMETA(DisplayName = "Frozen Green (below player)"),
};

// ─── Actor ─────────────────────────────────────────────────────────────────────
UCLASS()
class STACKOBOT_API AFallingDisk : public AActor
{
    GENERATED_BODY()

public:
    AFallingDisk();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Components ──────────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> DiskMesh;

    // ── Tuning (set by ADiskSpawner before use) ─────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Disk")
    float FallSpeed = 200.f;           // Unreal Units per second

    UPROPERTY(EditAnywhere, Category = "Disk")
    float KnockbackForce = 900.f;      // Force applied to airborne player on hit

    // ── Materials – assign in the Blueprint child class BP_FallingDisk ──────────
    // Leave these None in C++; you will pick them in the editor.
    UPROPERTY(EditDefaultsOnly, Category = "Disk|Visuals")
    TObjectPtr<UMaterialInterface> Mat_Falling;

    UPROPERTY(EditDefaultsOnly, Category = "Disk|Visuals")
    TObjectPtr<UMaterialInterface> Mat_FrozenRed;

    UPROPERTY(EditDefaultsOnly, Category = "Disk|Visuals")
    TObjectPtr<UMaterialInterface> Mat_FrozenGreen;

    // ── State (read by ADiskSpawner every tick) ─────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Disk")
    EDiskState DiskState = EDiskState::Falling;

    // ── Public API called by ADiskSpawner ───────────────────────────────────────

    /** Set starting speed and reset to Falling. Call before placing in the world. */
    void Initialize(float Speed);

    /** Freeze this disk as the one Red disk above the player. */
    void FreezeRed();

    /** Freeze this disk as a Green disk (at or below the player). */
    void FreezeGreen();

    /** Called by DiskSpawner when the player climbs above a Red disk. */
    void PromoteToGreen();

    /** Restore the fall speed that was active before this disk was frozen. */
    void Unfreeze();

    /** Teleport to NewLocation and re-initialize with a new speed (ground respawn). */
    void Respawn(FVector NewLocation, float NewSpeed);

private:
    float StoredFallSpeed = 200.f;   // Memorized when frozen; restored on Unfreeze

    void ApplyMaterial(UMaterialInterface* Mat);

    // Hit callback – wired in BeginPlay
    UFUNCTION()
    void OnDiskHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
                   UPrimitiveComponent* OtherComp, FVector NormalImpulse,
                   const FHitResult& Hit);
};
```

### `Source/StackOBot/FallingDisk.cpp`

```cpp
#include "FallingDisk.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"

// ─── Constructor ───────────────────────────────────────────────────────────────
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

// ─── Tick ──────────────────────────────────────────────────────────────────────
void AFallingDisk::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Only move when falling; frozen disks are stationary.
    if (DiskState == EDiskState::Falling)
    {
        AddActorWorldOffset(FVector(0.f, 0.f, -FallSpeed * DeltaTime));
    }
}

// ─── API ───────────────────────────────────────────────────────────────────────
void AFallingDisk::Initialize(float Speed)
{
    FallSpeed       = Speed;
    StoredFallSpeed = Speed;
    DiskState       = EDiskState::Falling;
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

// ─── Private ───────────────────────────────────────────────────────────────────
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
```

---

## Phase 3 – DiskSpawner Actor

The spawner owns the disk grid. It spawns one disk per cell at startup, watches
for ground hits to respawn disks, and applies the red/green freeze rules each tick.

### `Source/StackOBot/DiskSpawner.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FallingDisk.h"
#include "DiskSpawner.generated.h"

UCLASS()
class STACKOBOT_API ADiskSpawner : public AActor
{
    GENERATED_BODY()

public:
    ADiskSpawner();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // ── Grid configuration – tune these in the Details panel ────────────────────
    UPROPERTY(EditAnywhere, Category = "Spawner|Grid")
    int32 GridColumns = 4;

    UPROPERTY(EditAnywhere, Category = "Spawner|Grid")
    int32 GridRows = 4;

    /** Width and depth of each grid cell in Unreal Units (1 UU = 1 cm). */
    UPROPERTY(EditAnywhere, Category = "Spawner|Grid")
    float CellSize = 350.f;

    /** Z height above the actor's location where new disks appear. */
    UPROPERTY(EditAnywhere, Category = "Spawner|Grid")
    float SpawnHeightOffset = 3000.f;

    /** Disks that fall below this world Z are considered to have hit the ground. */
    UPROPERTY(EditAnywhere, Category = "Spawner|Grid")
    float GroundZ = 20.f;

    // ── Speed range ──────────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, Category = "Spawner|Speed")
    float MinSpeed = 80.f;

    UPROPERTY(EditAnywhere, Category = "Spawner|Speed")
    float MaxSpeed = 320.f;

    // ── Disk Blueprint child class (assign in Details panel) ─────────────────────
    // Set this to BP_FallingDisk (the Blueprint child you will create in Phase 6).
    UPROPERTY(EditDefaultsOnly, Category = "Spawner")
    TSubclassOf<AFallingDisk> DiskClass;

    // ── Called by UShootingComponent when its laser hits a disk ─────────────────
    void NotifyDiskHit(AFallingDisk* HitDisk, ACharacter* Player);

private:
    TArray<AFallingDisk*> Disks;          // Index = Col + Row * GridColumns
    AFallingDisk*         RedDisk = nullptr;   // At most one red disk at a time
    ACharacter*           CachedPlayer = nullptr;

    void  SpawnAllDisks();
    FVector CellSpawnLocation(int32 Col, int32 Row) const;
    float RandomSpeed() const;

    void CheckGroundHits();
    void CheckRedPromotion();
    void CheckGreenUnfreeze();
};
```

### `Source/StackOBot/DiskSpawner.cpp`

```cpp
#include "DiskSpawner.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"
#include "Math/UnrealMathUtility.h"

// ─── Constructor ───────────────────────────────────────────────────────────────
ADiskSpawner::ADiskSpawner()
{
    PrimaryActorTick.bCanEverTick = true;
    RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
}

// ─── BeginPlay ─────────────────────────────────────────────────────────────────
void ADiskSpawner::BeginPlay()
{
    Super::BeginPlay();

    CachedPlayer = Cast<ACharacter>(UGameplayStatics::GetPlayerCharacter(this, 0));
    SpawnAllDisks();
}

// ─── Tick ──────────────────────────────────────────────────────────────────────
void ADiskSpawner::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    CheckGroundHits();
    CheckRedPromotion();
    CheckGreenUnfreeze();
}

// ─── Public ────────────────────────────────────────────────────────────────────

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
    const float DiskZ   = HitDisk->GetActorLocation().Z;

    if (DiskZ > PlayerZ)
    {
        // Disk is above the player → freeze Red.
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
        // Disk is at or below the player → freeze Green immediately.
        HitDisk->FreezeGreen();
    }
}

// ─── Private ───────────────────────────────────────────────────────────────────
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
    const float GridWidth  = GridColumns * CellSize;
    const float GridDepth  = GridRows    * CellSize;

    const float OffsetX = (Col + 0.5f) * CellSize - GridWidth  * 0.5f;
    const float OffsetY = (Row + 0.5f) * CellSize - GridDepth  * 0.5f;

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
    const float DiskZ   = RedDisk->GetActorLocation().Z;

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
```

---

## Phase 4 – ShootingComponent

A `UActorComponent` you attach to `BP_Bot` in the Blueprint editor. It handles:
- ADS camera shift (lerps the Spring Arm socket offset)
- Crosshair widget show/hide
- Line-trace laser fire → notifies `ADiskSpawner`

### `Source/StackOBot/ShootingComponent.h`

```cpp
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShootingComponent.generated.h"

class ADiskSpawner;
class AFallingDisk;
class USpringArmComponent;
class UUserWidget;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class STACKOBOT_API UShootingComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UShootingComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType,
                               FActorComponentTickFunction* ThisTickFunction) override;

    // ── Tuning ──────────────────────────────────────────────────────────────────

    /** How far the laser can reach in Unreal Units. */
    UPROPERTY(EditAnywhere, Category = "Shooting")
    float TraceRange = 5000.f;

    /** Camera socket offset while aiming down sights (tune Y to shift right). */
    UPROPERTY(EditAnywhere, Category = "Shooting|ADS")
    FVector ADSSocketOffset = FVector(0.f, 80.f, 20.f);

    /** Speed of the camera lerp in/out of ADS. */
    UPROPERTY(EditAnywhere, Category = "Shooting|ADS")
    float ADSInterpSpeed = 8.f;

    /** Widget class for the crosshair. Create WBP_Crosshair in the editor and assign here. */
    UPROPERTY(EditDefaultsOnly, Category = "Shooting|ADS")
    TSubclassOf<UUserWidget> CrosshairWidgetClass;

    // ── Input handlers (call these from BP_Bot's Event Graph) ───────────────────

    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void StartADS();

    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void StopADS();

    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void Fire();

private:
    bool bIsADS = false;

    // Cached references found in BeginPlay
    TObjectPtr<USpringArmComponent> SpringArm;
    TObjectPtr<ADiskSpawner>        DiskSpawner;
    TObjectPtr<UUserWidget>         CrosshairWidget;

    FVector DefaultSocketOffset = FVector::ZeroVector;

    void ShowCrosshair();
    void HideCrosshair();
};
```

### `Source/StackOBot/ShootingComponent.cpp`

```cpp
#include "ShootingComponent.h"
#include "DiskSpawner.h"
#include "FallingDisk.h"

#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

// ─── Constructor ───────────────────────────────────────────────────────────────
UShootingComponent::UShootingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

// ─── BeginPlay ─────────────────────────────────────────────────────────────────
void UShootingComponent::BeginPlay()
{
    Super::BeginPlay();

    // ── Find the Spring Arm on the owning character ──────────────────────────────
    SpringArm = GetOwner()->FindComponentByClass<USpringArmComponent>();
    if (SpringArm)
    {
        DefaultSocketOffset = SpringArm->SocketOffset;
    }
    else
    {
        UE_LOG(LogTemp, Warning,
               TEXT("ShootingComponent: No USpringArmComponent found on %s"),
               *GetOwner()->GetName());
    }

    // ── Find the DiskSpawner placed in the level ─────────────────────────────────
    TArray<AActor*> Found;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADiskSpawner::StaticClass(), Found);
    if (Found.Num() > 0)
    {
        DiskSpawner = Cast<ADiskSpawner>(Found[0]);
    }
    else
    {
        UE_LOG(LogTemp, Warning,
               TEXT("ShootingComponent: No ADiskSpawner found in the level."));
    }

    // ── Create crosshair widget (hidden until ADS) ───────────────────────────────
    if (CrosshairWidgetClass)
    {
        APlayerController* PC = Cast<APlayerController>(
            Cast<APawn>(GetOwner())->GetController());
        if (PC)
        {
            CrosshairWidget = CreateWidget<UUserWidget>(PC, CrosshairWidgetClass);
        }
    }
}

// ─── Tick ──────────────────────────────────────────────────────────────────────
void UShootingComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Smoothly lerp the Spring Arm socket offset into/out of ADS position.
    if (SpringArm)
    {
        const FVector Target = bIsADS ? ADSSocketOffset : DefaultSocketOffset;
        SpringArm->SocketOffset = FMath::VInterpTo(
            SpringArm->SocketOffset, Target, DeltaTime, ADSInterpSpeed);
    }
}

// ─── Input Handlers ────────────────────────────────────────────────────────────
void UShootingComponent::StartADS()
{
    bIsADS = true;
    ShowCrosshair();
}

void UShootingComponent::StopADS()
{
    bIsADS = false;
    HideCrosshair();
}

/**
 * Fire performs a hitscan line trace from the camera centre.
 * A debug line is drawn as a placeholder laser; replace it with a Niagara beam
 * once you have set up the VFX asset.
 */
void UShootingComponent::Fire()
{
    APawn* OwnerPawn = Cast<APawn>(GetOwner());
    if (!OwnerPawn) return;

    APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController());
    if (!PC) return;

    // ── Determine trace start/end from the camera viewpoint ─────────────────────
    FVector  CamLoc;
    FRotator CamRot;
    PC->GetPlayerViewPoint(CamLoc, CamRot);

    const FVector TraceEnd = CamLoc + CamRot.Vector() * TraceRange;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());    // Don't hit ourselves

    FHitResult Hit;
    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit, CamLoc, TraceEnd, ECC_Visibility, QueryParams);

    // ── Placeholder laser visual (blue debug line, visible for 0.15 s) ──────────
    // TODO: Replace with a Niagara beam spawned at the character's weapon socket.
    DrawDebugLine(GetWorld(),
                  GetOwner()->GetActorLocation() + FVector(0, 0, 50),
                  bHit ? Hit.ImpactPoint : TraceEnd,
                  FColor::Cyan, false, 0.15f, 0, 3.f);

    // ── Notify the spawner if we hit a disk ──────────────────────────────────────
    if (bHit && DiskSpawner)
    {
        AFallingDisk* Disk = Cast<AFallingDisk>(Hit.GetActor());
        if (Disk)
        {
            DiskSpawner->NotifyDiskHit(Disk, Cast<ACharacter>(GetOwner()));
        }
    }
}

// ─── Private ───────────────────────────────────────────────────────────────────
void UShootingComponent::ShowCrosshair()
{
    if (CrosshairWidget && !CrosshairWidget->IsInViewport())
    {
        CrosshairWidget->AddToViewport();
    }
}

void UShootingComponent::HideCrosshair()
{
    if (CrosshairWidget && CrosshairWidget->IsInViewport())
    {
        CrosshairWidget->RemoveFromParent();
    }
}
```

---

## Phase 5 – Compile Everything

1. In Visual Studio: **Build → Build Solution** (`Ctrl+Shift+B`).
2. Fix any errors before proceeding. Common first-time mistakes:
   - Missing `#include` for a header → add the include shown in the code above.
   - `STACKOBOT_API` not recognized → make sure Build.cs was saved and the
     project was regenerated.
3. Once the build is clean, press **Play** in Visual Studio (or switch to Unreal
   and click **Compile** in the bottom toolbar) to hot-reload.

---

## Phase 6 – Blueprint Child Classes

C++ classes are rarely placed in the level directly. Create thin Blueprint
wrappers so you can assign meshes and materials in the editor.

### BP_FallingDisk

1. In the **Content Browser** navigate to `Content/StackOBot/Blueprints/`.
2. Right-click → **Blueprint Class** → search for and select `FallingDisk`
   (your C++ class) → name it `BP_FallingDisk`.
3. Open `BP_FallingDisk`. In the **Components** panel select `DiskMesh`.
   - Assign a **Static Mesh** (a flat cylinder works; Stack O Bot has disk-like
     meshes in `Content/StackOBot/Environment/`).
   - Set **Collision Preset** to `BlockAll` (should already be set from C++).
4. In the **Details** panel (with nothing selected) assign the three materials:
   - `Mat_Falling` → a neutral/default material
   - `Mat_FrozenRed` → a red emissive material
   - `Mat_FrozenGreen` → a green emissive material

   You can create simple materials in the editor:
   **Right-click Content Browser → Material → double-click to open → set Base
   Color to red/green → Save**.
5. **Compile and Save** `BP_FallingDisk`.

### BP_DiskSpawner

1. Right-click → **Blueprint Class** → select `DiskSpawner` → name it
   `BP_DiskSpawner`.
2. Open it. In the **Details** panel (Class Defaults view) set:
   - `Disk Class` → `BP_FallingDisk`
   - Adjust `GridColumns`, `GridRows`, `CellSize`, speed range as desired.
3. **Compile and Save**.

### WBP_Crosshair (Widget Blueprint)

1. Right-click Content Browser → **User Interface → Widget Blueprint** → name it
   `WBP_Crosshair`.
2. Open it. In the **Designer** tab drag a **Canvas Panel** onto the canvas if
   not already there.
3. Inside the Canvas Panel, add an **Image** widget.
   - Set its **Anchors** to center (the circle icon in the anchor preset picker).
   - Set **Alignment** to `0.5, 0.5` so it stays centered.
   - Set **Size** to about `32 x 32`.
   - Assign a crosshair texture (you can use any white texture and tint it, or
     import a PNG crosshair image).
4. **Compile and Save**.

---

## Phase 7 – Wire Input in BP_Bot

The three C++ functions (`StartADS`, `StopADS`, `Fire`) are `BlueprintCallable`
and must be called from `BP_Bot`'s Event Graph using Enhanced Input events.

### Step A – Add the ShootingComponent to BP_Bot

1. Open `BP_Bot` (found in
   `Content/StackOBot/Blueprints/Character/` or similar).
2. In the **Components** panel click **Add** → search for `ShootingComponent` →
   add it. It will appear as `ShootingComponent` in the list.
3. Select the component. In the **Details** panel assign:
   - `Crosshair Widget Class` → `WBP_Crosshair`
   - Tune `ADSSocketOffset` and `TraceRange` as needed.
4. **Compile and Save** `BP_Bot`.

### Step B – Create New Input Actions

1. In `Content/StackOBot/Input/` right-click → **Input → Input Action**.
   - Create `IA_ADS` with **Value Type = Digital (bool)**.
   - Create `IA_Fire` with **Value Type = Digital (bool)**.
2. Open the existing **Input Mapping Context** (likely `IMC_Default` in
   `Content/StackOBot/Input/`).
3. Add two new mappings:
   - `IA_ADS` → **Right Mouse Button**
   - `IA_Fire` → **Left Mouse Button**
4. Save the IMC.

### Step C – Bind Actions in BP_Bot's Event Graph

Open `BP_Bot`, go to the **Event Graph**.

Add the following nodes:

```
[Enhanced Input Action IA_ADS]
    Triggered  ──► [Get ShootingComponent] ──► [Start ADS]
    Completed  ──► [Get ShootingComponent] ──► [Stop ADS]

[Enhanced Input Action IA_Fire]
    Started    ──► [Get ShootingComponent] ──► [Fire]
```

To add an Enhanced Input event node: right-click in the graph → search for
`IA_ADS` → choose **Enhanced Input Action IA_ADS**. Repeat for `IA_Fire`.

To get the component: right-click → **Get ShootingComponent** (it appears because
you added it in Step A).

**Compile and Save** `BP_Bot`.

---

## Phase 8 – Level Setup

### Place the DiskSpawner

1. Open your level `LVL_New`.
2. Drag `BP_DiskSpawner` from the Content Browser into the viewport. Place it
   above the center of the play area (e.g., X=0, Y=0, Z=0 — its
   `SpawnHeightOffset` property lifts the disks above it automatically).
3. In the **Details** panel configure the spawner:
   - `Grid Columns` / `Grid Rows` — start with 3×3 or 4×4.
   - `Cell Size` — must be larger than the disk mesh diameter so disks don't
     overlap. `350` cm is a good starting point.
   - `Spawn Height Offset` — how high above the spawner the disks first appear.
     `3000` cm means disks start 30 m up and fall down.
   - `Ground Z` — set to just above your floor mesh height (e.g., `20`).
   - `Min Speed` / `Max Speed` — `80` / `300` gives good variety.

### Add a Win Zone (Blueprint Trigger Volume)

1. In the **Place Actors** panel (or the Content Browser) find **Trigger Volume**
   and drag one high in the level (e.g., Z = 4000–5000 cm).
2. Resize it to cover the target area.
3. In the **Details** panel click **+** next to **On Actor Begin Overlap**.
4. In the Event Graph that opens, connect the overlap event to:
   - **Get Game Mode** → **Cast to GM_InGame** → call the appropriate win
     function, OR
   - Simply **Open Level** to a win screen, OR
   - **Print String** "You Win!" as a placeholder while testing.

### Verify World Settings

Window → World Settings → confirm:
- **GameMode Override** = `GM_InGame`
- **Default Pawn Class** = `BP_Bot`

### Play and Test

Hit **Play** in the editor. Expected behavior:
- Disks spawn above and fall toward the ground.
- Hold RMB: camera shifts right, crosshair appears.
- LMB while aiming at a disk above you: it turns red and stops.
- LMB while aiming at a disk below you: it turns green.
- Jump onto a frozen disk (it is solid — `BlockAll` collision).
- Climb above the red disk: it turns green automatically.
- Fall below any green disk: it resumes falling.
- A falling disk hitting you while airborne launches you downward.
- Reach the Trigger Volume at the top: win condition fires.

---

## Troubleshooting Reference

| Symptom | Likely Cause | Fix |
|---|---|---|
| Disks don't spawn | `DiskClass` not set on `BP_DiskSpawner` | Open `BP_DiskSpawner` Details, set `Disk Class` to `BP_FallingDisk` |
| Disk mesh is invisible | No mesh assigned on `BP_FallingDisk` | Open `BP_FallingDisk`, select `DiskMesh` component, assign a Static Mesh |
| Colors don't change | Materials not assigned | Assign `Mat_Falling`, `Mat_FrozenRed`, `Mat_FrozenGreen` in `BP_FallingDisk` Details |
| Laser does nothing | No `BP_DiskSpawner` in level | Drag `BP_DiskSpawner` into the level |
| Crosshair never appears | `CrosshairWidgetClass` not set | Select `ShootingComponent` on `BP_Bot`, assign `WBP_Crosshair` |
| Camera does not shift on RMB | `IA_ADS` not in IMC, or not bound in BP_Bot Event Graph | Follow Phase 7 Step B and C |
| Player falls through frozen disk | Disk collision not `BlockAll` | Confirm `DiskMesh` collision preset in `BP_FallingDisk` |
| `STACKOBOT_API` compile error | Build.cs not saved or VS project not regenerated | Save Build.cs, right-click .uproject → Generate VS project files, rebuild |

---

## What Was Simplified vs Idea.md

| Idea.md Detail | This Guide |
|---|---|
| Strict "exactly 1 disk per cell at all times" | Disks respawn at their original cell when they hit the ground — minor timing gaps are possible but invisible during play |
| Camera moves "directly behind → right" with exact offset | Spring Arm `SocketOffset` lerped to `ADSSocketOffset`; exact values are tunable in the Details panel |
| Blue laser beam visual | `DrawDebugLine` placeholder; replace with a Niagara beam asset once art is ready |
| Disk grid bounded by squares in XY | Grid cells computed from `CellSize`; disks are placed at cell centers |
