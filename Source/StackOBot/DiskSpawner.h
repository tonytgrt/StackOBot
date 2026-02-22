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

    // ©¤©¤ Grid configuration ¨C tune these in the Details panel ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
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

    // ©¤©¤ Speed range ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
    UPROPERTY(EditAnywhere, Category = "Spawner|Speed")
    float MinSpeed = 80.f;

    UPROPERTY(EditAnywhere, Category = "Spawner|Speed")
    float MaxSpeed = 320.f;

    // ©¤©¤ Disk Blueprint child class (assign in Details panel) ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
    // Set this to BP_FallingDisk (the Blueprint child you will create in Phase 6).
    UPROPERTY(EditDefaultsOnly, Category = "Spawner")
    TSubclassOf<AFallingDisk> DiskClass;

    // ©¤©¤ Called by UShootingComponent when its laser hits a disk ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
    void NotifyDiskHit(AFallingDisk* HitDisk, ACharacter* Player);

private:
    TArray<AFallingDisk*> Disks;          // Index = Col + Row * GridColumns
    AFallingDisk* RedDisk = nullptr;   // At most one red disk at a time
    ACharacter* CachedPlayer = nullptr;

    void  SpawnAllDisks();
    FVector CellSpawnLocation(int32 Col, int32 Row) const;
    float RandomSpeed() const;

    void CheckGroundHits();
    void CheckRedPromotion();
    void CheckGreenUnfreeze();
};