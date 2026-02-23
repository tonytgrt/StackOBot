#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ShootingComponent.generated.h"

class ADiskSpawner;
class AFallingDisk;
class USpringArmComponent;
class UUserWidget;
class UNiagaraSystem;

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

    // ���� Tuning ������������������������������������������������������������������������������������������������������������������������������������

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

    /** Niagara beam system for the laser visual. Assign NS_LaserBeam here. */
    UPROPERTY(EditDefaultsOnly, Category = "Shooting")
    TObjectPtr<UNiagaraSystem> LaserEffect;

    // ���� Input handlers (call these from BP_Bot's Event Graph) ��������������������������������������

    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void StartADS();

    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void StopADS();

    UFUNCTION(BlueprintCallable, Category = "Shooting")
    void Fire();

private:
    bool bIsADS = false;

    // Cached references found in BeginPlay — UPROPERTY() keeps them from being GC'd
    UPROPERTY()
    TObjectPtr<USpringArmComponent> SpringArm;

    UPROPERTY()
    TObjectPtr<ADiskSpawner>        DiskSpawner;

    UPROPERTY()
    TObjectPtr<UUserWidget>         CrosshairWidget;

    FVector DefaultSocketOffset = FVector::ZeroVector;

    void ShowCrosshair();
    void HideCrosshair();
};