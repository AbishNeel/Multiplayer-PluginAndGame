// Fill out your copyright notice in the Description page of Project Settings.


#include "OverheadWidget.h"
#include "Components/TextBlock.h"
#include "GameFramework/PlayerState.h"

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* InLevel, UWorld* InWorld)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(InLevel, InWorld);
}

void UOverheadWidget::SetDisplayText(FString Text)
{
	if (DisplayText == NULL)
		return;
	DisplayText->SetText(FText::FromString(Text));
}

void UOverheadWidget::ShowPlayerName(APawn* Pawn)
{
	APlayerState* PlayerState = Pawn->GetPlayerState();
	if (PlayerState)
		SetDisplayText(PlayerState->GetPlayerName());
}