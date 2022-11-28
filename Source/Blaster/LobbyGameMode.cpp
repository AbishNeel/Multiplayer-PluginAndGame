// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameStateBase.h"

void ALobbyGameMode::BeginPlay()
{
	Super::BeginPlay();

	/*UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();*/

	GetWorldTimerManager().SetTimer(LobbyTimer, this, &ThisClass::StartSession, 50.f, false);
}

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (GameState)
	{
		int32 NumPlayers = GameState->PlayerArray.Num();
		if (NumPlayers == 3)
		{
			//GetWorldTimerManager().ClearTimer(LobbyTimer);
			StartSession();
		}
	}
}

void ALobbyGameMode::Logout(AController* Exiting)
{

}

void ALobbyGameMode::StartSession()
{
	UWorld* World = GetWorld();
	if (World)
	{
		bUseSeamlessTravel = true;
		World->ServerTravel("/Game/Maps/MainMap?listen");
	}
}