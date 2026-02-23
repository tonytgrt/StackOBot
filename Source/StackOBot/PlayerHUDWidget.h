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
    /** Called every tick by ADiskSpawner to refresh both lines. */
    void UpdateValues(float CurrentZ, float HighestZ);

    /** Called once from BeginPlay to set the static goal line. */
    void SetGoalText(float GoalHeight);

private:
    // Looks up a TextBlock by widget name at runtime — avoids BindWidget
    // serialization issues in packaged Shipping builds.
    void SetBlockText(FName WidgetName, const FString& Text);
};
