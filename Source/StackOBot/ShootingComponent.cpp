#include "ShootingComponent.h"
#include "DiskSpawner.h"
#include "FallingDisk.h"

#include "GameFramework/Character.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/PlayerController.h"
#include "Blueprint/UserWidget.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"
#include "DrawDebugHelpers.h"

// ������ Constructor ������������������������������������������������������������������������������������������������������������������������������
UShootingComponent::UShootingComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

// ������ BeginPlay ����������������������������������������������������������������������������������������������������������������������������������
void UShootingComponent::BeginPlay()
{
    Super::BeginPlay();

    // ���� Find the Spring Arm on the owning character ������������������������������������������������������������
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

    // ���� Find the DiskSpawner placed in the level ������������������������������������������������������������������
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

    // ���� Create crosshair widget (hidden until ADS) ��������������������������������������������������������������
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

// ������ Tick ��������������������������������������������������������������������������������������������������������������������������������������������
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

// ������ Input Handlers ������������������������������������������������������������������������������������������������������������������������
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

    // ���� Determine trace start/end from the camera viewpoint ������������������������������������������
    FVector  CamLoc;
    FRotator CamRot;
    PC->GetPlayerViewPoint(CamLoc, CamRot);

    const FVector TraceEnd = CamLoc + CamRot.Vector() * TraceRange;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(GetOwner());    // Don't hit ourselves

    FHitResult Hit;
    const bool bHit = GetWorld()->LineTraceSingleByChannel(
        Hit, CamLoc, TraceEnd, ECC_Visibility, QueryParams);

    // ←─ Niagara laser beam visual ──────────────────────────────────────────────────
    if (LaserEffect)
    {
        // Beam starts at the character's chest/front (visually connected to mesh).
        const FVector BeamStart = GetOwner()->GetActorLocation()
            + GetOwner()->GetActorForwardVector() * 30.f
            + FVector(0.f, 0.f, 60.f);

        // Beam end: project from BeamStart along the CAMERA's forward direction by
        // the hit distance. This keeps the visual beam aligned with the crosshair
        // even though BeamStart != CamLoc (standard third-person technique).
        const float HitDist = bHit
            ? FVector::Dist(CamLoc, Hit.ImpactPoint)
            : TraceRange;
        const FVector BeamEnd = BeamStart + CamRot.Vector() * HitDist;

        UNiagaraComponent* Beam = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(), LaserEffect, BeamStart);
        if (Beam)
        {
            Beam->SetVectorParameter(FName("BeamEnd"), BeamEnd);
        }
    }

    // ���� Notify the spawner if we hit a disk ����������������������������������������������������������������������������
    if (bHit && DiskSpawner)
    {
        AFallingDisk* Disk = Cast<AFallingDisk>(Hit.GetActor());
        if (Disk)
        {
            DiskSpawner->NotifyDiskHit(Disk, Cast<ACharacter>(GetOwner()));
        }
    }
}

// ������ Private ��������������������������������������������������������������������������������������������������������������������������������������
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