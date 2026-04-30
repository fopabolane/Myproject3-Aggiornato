// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/TextBlock.h"
#include "Blueprint/UserWidget.h"
#include "GameHUDWidget.generated.h"

/**
 * 
 */
UCLASS()
class MYPROJECT3_API UGameHUDWidget : public UUserWidget
{
	GENERATED_BODY()
	

public:

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TurnText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* PlayerUnitsText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* AIUnitsText;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* TowerText;

    void UpdateHUD(
        const FString& CurrentTurn,
        const FString& PlayerInfo,
        const FString& AIInfo,
        const FString& TowerInfo
    );
};
