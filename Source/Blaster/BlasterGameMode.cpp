// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterGameMode.h"
#include "Character/BlasterCharacter.h"
#include "Kismet/Gameplaystatics.h"
#include "GameFramework/PlayerStart.h"
#include "Character/BlasterPlayerController.h"

ABlasterGameMode::ABlasterGameMode()
{
	bDelayedStart = true;
}

void ABlasterGameMode::BeginPlay()
{
	Super::BeginPlay();

	LevelStartTime = GetWorld()->GetTimeSeconds();
}

void ABlasterGameMode::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MatchState == MatchState::WaitingToStart)
	{
		CountdownTime = WarmupTime - GetWorld()->GetTimeSeconds() + LevelStartTime;
		if (CountdownTime <= 0.f)
			StartMatch();
	}
}

void ABlasterGameMode::PlayerEliminated(class ABlasterCharacter* ElimmedPlayer, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController)
{
	ABlasterPlayerState* AttackerState = AttackerController ? AttackerController->GetPlayerState<ABlasterPlayerState>() : NULL;
	ABlasterPlayerState* VictimState = VictimController ? VictimController->GetPlayerState<ABlasterPlayerState>() : NULL;
	if (AttackerState && AttackerState != VictimState)
		AttackerState->AddToScore(1.f);
	if (VictimState && VictimState != AttackerState)
		VictimState->AddToDeaths(1);

	OnPlayerEliminated.Broadcast(AttackerState, VictimState);

	if(ElimmedPlayer)
		ElimmedPlayer->Elim();
}

void ABlasterGameMode::RequestRespawn(ACharacter* EliminatedPlayer, AController* EliminatedController)
{
	if (EliminatedPlayer)
	{
		EliminatedPlayer->Reset();
		EliminatedPlayer->Destroy();
	}
	if (EliminatedController)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(this, APlayerStart::StaticClass(), PlayerStarts);
		int32 SpawnInt = FMath::RandRange(0, PlayerStarts.Num() - 1);
		RestartPlayerAtPlayerStart(EliminatedController, PlayerStarts[SpawnInt]);
	}
}