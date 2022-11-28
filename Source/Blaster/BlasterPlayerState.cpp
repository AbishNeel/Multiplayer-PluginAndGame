// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerState.h"
#include "Character/BlasterCharacter.h"
#include "Character/BlasterPlayerController.h"
#include "Net/UnrealNetwork.h"

void ABlasterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ABlasterPlayerState, Deaths);
}

void ABlasterPlayerState::AddToScore(float ScoreAmount)
{
	SetScore(GetScore() + ScoreAmount);
	Character = Character == NULL ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == NULL ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
			Controller->SetHUDScore(GetScore());
	}
}

void ABlasterPlayerState::OnRep_Score()
{
	Super::OnRep_Score();

	Character = Character == NULL ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == NULL ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
			Controller->SetHUDScore(GetScore());
	}
}

void ABlasterPlayerState::AddToDeaths(int32 DeathsAmount)
{
	Deaths += DeathsAmount;
	Character = Character == NULL ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == NULL ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
			Controller->SetHUDDeaths(Deaths);
	}
}

void ABlasterPlayerState::OnRep_Deaths()
{
	Character = Character == NULL ? Cast<ABlasterCharacter>(GetPawn()) : Character;
	if (Character)
	{
		Controller = Controller == NULL ? Cast<ABlasterPlayerController>(Character->Controller) : Controller;
		if (Controller)
			Controller->SetHUDDeaths(Deaths);
	}
}