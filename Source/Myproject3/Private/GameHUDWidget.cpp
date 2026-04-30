// Fill out your copyright notice in the Description page of Project Settings.


#include "GameHUDWidget.h"

void UGameHUDWidget::UpdateHUD(
    const FString& CurrentTurn,
    const FString& PlayerInfo,
    const FString& AIInfo,
    const FString& TowerInfo)
{
    if (TurnText)
    {
        TurnText->SetText(FText::FromString(CurrentTurn));
    }

    if (PlayerUnitsText)
    {
        PlayerUnitsText->SetText(FText::FromString(PlayerInfo));
    }

    if (AIUnitsText)
    {
        AIUnitsText->SetText(FText::FromString(AIInfo));
    }

    if (TowerText)
    {
        TowerText->SetText(FText::FromString(TowerInfo));
    }
}