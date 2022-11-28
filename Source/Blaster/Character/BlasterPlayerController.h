// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "BlasterPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class BLASTER_API ABlasterPlayerController : public APlayerController
{
	GENERATED_BODY()

private:
	UPROPERTY()
	class ABlasterHUD* BlasterHUD;

	UPROPERTY()
	class UCharacterOverlay* CharacterOverlay;

	FTimerHandle ElimDisplayTimer;
	
	uint32 CountDownInt = 0;
	float MatchTime = 120.f;


	void StopElimDisplay();
	void SetMatchTime();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	/**
	* Sync time between client and server
	*/
	UPROPERTY(EditAnywhere, Category = "Time")
	float SyncTimeInterval = 5.f;

	float ServerDelta;
	FTimerHandle TimeSyncTimer;

	float GetServerTime();
	void SyncTime();

	UFUNCTION(Server, Reliable)
	void ServerRequestServerTime(float TimeOfClientRequest);

	UFUNCTION(Client, Reliable)
	void ClientReportServerTime(float TimeOfClientRequest, float TimeServerRecievedClientRequest);

public:
	void SetHUDHealth(float Health, float MaxHealth);
	void SetHUDScore(float Score);
	void SetHUDDeaths(int32 Deaths);
	void SetHUDWeaponAmmo(int32 Ammo);
	void SetHUDCarriedAmmo(int32 Ammo);
	void SetWeaponTypeText(FLinearColor TextColor, FString WeaponTypeText);
	void DisplayElimText(class ABlasterPlayerState* AttackerState, ABlasterPlayerState* VictimState);
	void DisplayMatchTimer(float CountdownTime);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastDisplayElimText(ABlasterPlayerState* AttackerState, ABlasterPlayerState* VictimState);
	virtual void OnPossess(APawn* InPawn) override;
};
