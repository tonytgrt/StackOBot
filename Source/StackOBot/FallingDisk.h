#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FallingDisk.generated.h"

// ©¤©¤©¤ Disk state ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
UENUM(BlueprintType)
enum class EDiskState : uint8
{
    Falling      UMETA(DisplayName = "Falling"),
    FrozenRed    UMETA(DisplayName = "Frozen Red  (one above player)"),
    FrozenGreen  UMETA(DisplayName = "Frozen Green (below player)"),
};

// ©¤©¤©¤ Actor ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
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

    // ©¤©¤ Components ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TObjectPtr<UStaticMeshComponent> DiskMesh;

    // ©¤©¤ Tuning ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Disk")
    float FallSpeed = 200.f;           // Unreal Units per second

    UPROPERTY(EditAnywhere, Category = "Disk")
    float KnockbackForce = 900.f;      // Force applied to airborne player on hit

    // ©¤©¤ Materials ¨C assign in the Blueprint child class BP_FallingDisk ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
    UPROPERTY(EditDefaultsOnly, Category = "Disk|Visuals")
    TObjectPtr<UMaterialInterface> Mat_Falling;

    UPROPERTY(EditDefaultsOnly, Category = "Disk|Visuals")
    TObjectPtr<UMaterialInterface> Mat_FrozenRed;

    UPROPERTY(EditDefaultsOnly, Category = "Disk|Visuals")
    TObjectPtr<UMaterialInterface> Mat_FrozenGreen;

    // ©¤©¤ State (read by ADiskSpawner every tick) ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Disk")
    EDiskState DiskState = EDiskState::Falling;

    // ©¤©¤ Public API called by ADiskSpawner ©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤©¤

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

    // Hit callback ¨C wired in BeginPlay
    UFUNCTION()
    void OnDiskHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse,
        const FHitResult& Hit);
};