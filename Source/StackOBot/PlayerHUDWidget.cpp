#include "PlayerHUDWidget.h"
#include "Components/TextBlock.h"

void UPlayerHUDWidget::SetBlockText(FName WidgetName, const FString& Text)
{
    UTextBlock* TB = Cast<UTextBlock>(GetWidgetFromName(WidgetName));
    if (TB)
        TB->SetText(FText::FromString(Text));
}

void UPlayerHUDWidget::SetGoalText(float GoalHeight)
{
    SetBlockText(TEXT("Text_WinGoal"),
        FString::Printf(TEXT("Reach %.0f to win"), GoalHeight));
}

void UPlayerHUDWidget::UpdateValues(float CurrentZ, float HighestZ)
{
    SetBlockText(TEXT("Text_CurrentZ"),
        FString::Printf(TEXT("Height : %.0f"), CurrentZ));
    SetBlockText(TEXT("Text_HighestZ"),
        FString::Printf(TEXT("Best    : %.0f"), HighestZ));
}
