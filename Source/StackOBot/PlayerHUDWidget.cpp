#include "PlayerHUDWidget.h"
#include "Components/TextBlock.h"

void UPlayerHUDWidget::SetGoalText(float GoalHeight)
{
    if (Text_WinGoal)
    {
        Text_WinGoal->SetText(
            FText::FromString(FString::Printf(TEXT("Reach %.0f to win"), GoalHeight)));
    }
}

void UPlayerHUDWidget::UpdateValues(float CurrentZ, float HighestZ)
{
    if (Text_CurrentZ)
    {
        Text_CurrentZ->SetText(
            FText::FromString(FString::Printf(TEXT("Height : %.0f"), CurrentZ)));
    }
    if (Text_HighestZ)
    {
        Text_HighestZ->SetText(
            FText::FromString(FString::Printf(TEXT("Best    : %.0f"), HighestZ)));
    }
}