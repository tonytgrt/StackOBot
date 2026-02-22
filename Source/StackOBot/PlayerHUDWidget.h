#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerHUDWidget.generated.h"

class UTextBlock;

UCLASS()
class STACKOBOT_API UPlayerHUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    /**
     * These TextBlock names must match exactly what you name the widgets
     * inside WBP_PlayerHUD in the Designer tab.
     */
    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_CurrentZ;

    UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
    TObjectPtr<UTextBlock> Text_HighestZ;

    /** Called every tick by ADiskSpawner to refresh both lines. */
    void UpdateValues(float CurrentZ, float HighestZ);
};