// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "BlasterPlayerState.h"
#include "BlasterGameMode.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterGameMode : public AGameMode
{
	GENERATED_BODY()

	DECLARE_MULTICAST_DELEGATE_TwoParams(FPlayerEliminated, ABlasterPlayerState* AttackerState, ABlasterPlayerState* VictimState);

public:
	ABlasterGameMode();
	virtual void PlayerEliminated(class ABlasterCharacter* ElimmedPlayer, class ABlasterPlayerController* VictimController, ABlasterPlayerController* AttackerController);
	void RequestRespawn(ACharacter* EliminatedPlayer, AController* EliminatedController);
	
	FPlayerEliminated OnPlayerEliminated;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

private:

	UPROPERTY(EditAnywhere, Category = "Match Start")
	float WarmupTime = 10.f;

	float LevelStartTime;
	float CountdownTime;
};
