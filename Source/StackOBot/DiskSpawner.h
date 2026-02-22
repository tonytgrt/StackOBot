#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FallingDisk.h"
#include "DiskSpawner.generated.h"

class UPlayerHUDWidget;   // forward declare ¨C defined in Phase 3b

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

    // ©¤©¤ Grid configuration ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
    UPROPERTY(EditAnywhere, Category = "Spawner|Grid")
    int32 GridColumns = 4;

    UPROPERTY(EditAnywhere, Category = "Spawner|Grid")
    int32 GridRows = 4;

    /** Width and depth of each grid cell in Unreal Units (1 UU = 1 cm). */
    UPROPERTY(EditAnywhere, Category = "Spawner|Grid")
    float CellSize = 350.f;

    /** Disks spawn this many units above the player's current Z each time they respawn. */
    UPROPERTY(EditAnywhere, Category = "Spawner|Grid")
    float SpawnHeightOffset = 3000.f;

    /** Hard floor: disks below this world-Z are always respawned (safety net). */
    UPROPERTY(EditAnywhere, Category = "Spawner|Grid")
    float GroundZ = -123.f;

    /** Disks that drop more than this many units below the player are also respawned. */
    UPROPERTY(EditAnywhere, Category = "Spawner|Grid")
    float RespawnBelowOffset = 800.f;

    // ©¤©¤ Speed range ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
    UPROPERTY(EditAnywhere, Category = "Spawner|Speed")
    float MinSpeed = 150.f;

    UPROPERTY(EditAnywhere, Category = "Spawner|Speed")
    float MaxSpeed = 500.f;

    // ©¤©¤ Win condition ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
    /** Player wins when their Z position exceeds this value. */
    UPROPERTY(EditAnywhere, Category = "Spawner|Win")
    float WinZ = 1000.f;

    /** Assign UI_WinScreen here in BP_DiskSpawner's Class Defaults. */
    UPROPERTY(EditDefaultsOnly, Category = "Spawner|Win")
    TSubclassOf<UUserWidget> WinScreenClass;

    // ©¤©¤ HUD ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
    /** Assign WBP_PlayerHUD here in BP_DiskSpawner's Class Defaults. */
    UPROPERTY(EditDefaultsOnly, Category = "Spawner|HUD")
    TSubclassOf<UPlayerHUDWidget> HUDWidgetClass;

    // ©¤©¤ Disk class ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
    UPROPERTY(EditDefaultsOnly, Category = "Spawner")
    TSubclassOf<AFallingDisk> DiskClass;

    // ©¤©¤ Public API ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
    /** Called by UShootingComponent when its laser hits a disk. */
    void NotifyDiskHit(AFallingDisk* HitDisk, ACharacter* Player);

    /** Called by UI_WinScreen's "Play Infinite Mode" button. Disables win check and resumes play. */
    UFUNCTION(BlueprintCallable, Category = "Spawner")
    void StartInfiniteMode();

private:
    TArray<AFallingDisk*> Disks;
    AFallingDisk* RedDisk = nullptr;

    // UPROPERTY keeps these from being garbage-collected mid-session
    UPROPERTY()
    TObjectPtr<ACharacter> CachedPlayer;

    UPROPERTY()
    TObjectPtr<UPlayerHUDWidget> HUDWidget;

    UPROPERTY()
    TObjectPtr<UUserWidget> WinScreenWidget;

    bool  bGameWon = false;
    bool  bInfiniteMode = false;
    float HighestZ = 0.f;

    void    SpawnAllDisks();
    FVector CellSpawnLocation(int32 Col, int32 Row) const;
    float   RandomSpeed() const;

    void CheckGroundHits();
    void CheckRedPromotion();
    void CheckGreenUnfreeze();
    void CheckWinCondition(float PlayerZ);
    void UpdateHUD(float PlayerZ);
    void ShowWinScreen();
};