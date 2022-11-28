// Fill out your copyright notice in the Description page of Project Settings.


#include "BlasterPlayerController.h"
#include "../HUD/BlasterHUD.h"
#include "../HUD/CharacterOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "BlasterCharacter.h"
#include "../BlasterPlayerState.h"
#include "../BlasterGameMode.h"
#include "TimerManager.h"

void ABlasterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	BlasterHUD = Cast<ABlasterHUD>(GetHUD());

	GetWorldTimerManager().SetTimer(TimeSyncTimer, this, &ThisClass::SyncTime, 5.f, true, 1.f);

	ABlasterGameMode* BlasterGameMode = GetWorld()->GetAuthGameMode<ABlasterGameMode>();
	if (BlasterGameMode)
		BlasterGameMode->OnPlayerEliminated.AddUObject(this, &ThisClass::DisplayElimText);
}

void ABlasterPlayerController::Tick(float DeltaTime)
{
	SetMatchTime();
}

void ABlasterPlayerController::SyncTime()
{
	if (IsLocalController())
		ServerRequestServerTime(GetWorld()->GetTimeSeconds());
}

void ABlasterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	ABlasterCharacter* BlasterCharacter = Cast<ABlasterCharacter>(InPawn);
	if (BlasterCharacter)
		SetHUDHealth(BlasterCharacter->GetHealth(), BlasterCharacter->GetMaxHealth());
}

void ABlasterPlayerController::SetHUDHealth(float Health, float MaxHealth)
{
	BlasterHUD = BlasterHUD == NULL ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bIsValid = BlasterHUD
		&& BlasterHUD->CharacterOverlay
		&& BlasterHUD->CharacterOverlay->HealthBar
		&& BlasterHUD->CharacterOverlay->HealthText;
	if (bIsValid)
	{
		const float HealthPercent = Health / MaxHealth;
		BlasterHUD->CharacterOverlay->HealthBar->SetPercent(HealthPercent);
		
		FString HealthText = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), FMath::CeilToInt(MaxHealth));
		BlasterHUD->CharacterOverlay->HealthText->SetText(FText::FromString(HealthText));
	}
}

void ABlasterPlayerController::SetHUDScore(float Score)
{
	BlasterHUD = BlasterHUD == NULL ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bIsValid = BlasterHUD
		&& BlasterHUD->CharacterOverlay
		&& BlasterHUD->CharacterOverlay->ScoreAmount;
	if (bIsValid)
	{
		FString ScoreText = FString::Printf(TEXT("%d"), FMath::FloorToInt(Score));
		BlasterHUD->CharacterOverlay->ScoreAmount->SetText(FText::FromString(ScoreText));
	}
}

void ABlasterPlayerController::SetHUDDeaths(int32 Deaths)
{
	BlasterHUD = BlasterHUD == NULL ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bIsValid = BlasterHUD
		&& BlasterHUD->CharacterOverlay
		&& BlasterHUD->CharacterOverlay->DeathsAmount;
	if (bIsValid)
	{
		FString DeathsText = FString::Printf(TEXT("%d"), Deaths);
		BlasterHUD->CharacterOverlay->DeathsAmount->SetText(FText::FromString(DeathsText));
	}
}

void ABlasterPlayerController::SetHUDWeaponAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == NULL ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bIsValid = BlasterHUD
		&& BlasterHUD->CharacterOverlay
		&& BlasterHUD->CharacterOverlay->WeaponAmmoAmount;
	if (bIsValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->WeaponAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABlasterPlayerController::SetHUDCarriedAmmo(int32 Ammo)
{
	BlasterHUD = BlasterHUD == NULL ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bIsValid = BlasterHUD
		&& BlasterHUD->CharacterOverlay
		&& BlasterHUD->CharacterOverlay->CarriedAmmoAmount;
	if (bIsValid)
	{
		FString AmmoText = FString::Printf(TEXT("%d"), Ammo);
		BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetText(FText::FromString(AmmoText));
	}
}

void ABlasterPlayerController::SetWeaponTypeText(FLinearColor TextColor, FString WeaponTypeText)
{
	BlasterHUD = BlasterHUD == NULL ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bIsValid = BlasterHUD
		&& BlasterHUD->CharacterOverlay
		&& BlasterHUD->CharacterOverlay->WeaponType;
	if (bIsValid)
	{
		FSlateColor WeaponTextColor = FSlateColor(TextColor);

		BlasterHUD->CharacterOverlay->WeaponType->SetText(FText::FromString(WeaponTypeText));
		BlasterHUD->CharacterOverlay->WeaponType->SetColorAndOpacity(WeaponTextColor);
		if (BlasterHUD->CharacterOverlay->CarriedAmmoAmount)
			BlasterHUD->CharacterOverlay->CarriedAmmoAmount->SetColorAndOpacity(WeaponTextColor);
	}
}

void ABlasterPlayerController::DisplayElimText(ABlasterPlayerState* AttackerState, ABlasterPlayerState* VictimState)
{
	MulticastDisplayElimText(AttackerState, VictimState);
}

void ABlasterPlayerController::MulticastDisplayElimText_Implementation(ABlasterPlayerState* AttackerState, ABlasterPlayerState* VictimState)
{
	bool bIsValid = BlasterHUD
		&& BlasterHUD->CharacterOverlay
		&& BlasterHUD->CharacterOverlay->ElimText1
		&& BlasterHUD->CharacterOverlay->ElimText2
		&& BlasterHUD->CharacterOverlay->ElimText3
		&& BlasterHUD->CharacterOverlay->ElimText4;
	if (bIsValid)
	{
		GetWorldTimerManager().ClearTimer(ElimDisplayTimer);

		FText TextBox2 = BlasterHUD->CharacterOverlay->ElimText1->GetText();
		FText TextBox3 = BlasterHUD->CharacterOverlay->ElimText2->GetText();
		FText TextBox4 = BlasterHUD->CharacterOverlay->ElimText3->GetText();

		BlasterHUD->CharacterOverlay->ElimText4->SetText(TextBox4);
		BlasterHUD->CharacterOverlay->ElimText3->SetText(TextBox3);
		BlasterHUD->CharacterOverlay->ElimText2->SetText(TextBox2);

		if (AttackerState && VictimState)
		{
			FString TextBox1 = FString::Printf(TEXT("%s eliminated %s"), *AttackerState->GetPlayerName(), *VictimState->GetPlayerName());
			BlasterHUD->CharacterOverlay->ElimText1->SetText(FText::FromString(TextBox1));
		}

		GetWorldTimerManager().SetTimer(ElimDisplayTimer, this, &ThisClass::StopElimDisplay, 0.5f, true, 3.f);
	}
}

void ABlasterPlayerController::StopElimDisplay()
{
	FString EmptyString("");
	if (BlasterHUD->CharacterOverlay->ElimText4->GetText().ToString() != EmptyString)
	{
		BlasterHUD->CharacterOverlay->ElimText4->SetText(FText::FromString(EmptyString));
		return;
	}
	if (BlasterHUD->CharacterOverlay->ElimText3->GetText().ToString() != EmptyString)
	{
		BlasterHUD->CharacterOverlay->ElimText3->SetText(FText::FromString(EmptyString));
		return;
	}
	if (BlasterHUD->CharacterOverlay->ElimText2->GetText().ToString() != EmptyString)
	{
		BlasterHUD->CharacterOverlay->ElimText2->SetText(FText::FromString(EmptyString));
		return;
	}
	if (BlasterHUD->CharacterOverlay->ElimText1->GetText().ToString() != EmptyString)
	{
		BlasterHUD->CharacterOverlay->ElimText1->SetText(FText::FromString(EmptyString));
		GetWorldTimerManager().ClearTimer(ElimDisplayTimer);
		return;
	}
}

void ABlasterPlayerController::DisplayMatchTimer(float CountdownTime)
{
	BlasterHUD = BlasterHUD == NULL ? Cast<ABlasterHUD>(GetHUD()) : BlasterHUD;
	bool bIsValid = BlasterHUD
		&& BlasterHUD->CharacterOverlay
		&& BlasterHUD->CharacterOverlay->MatchTimerText;
	if (bIsValid)
	{
		int32 Minutes = FMath::FloorToInt(CountdownTime / 60.f);
		int32 Seconds = FMath::FloorToInt(CountdownTime - Minutes * 60);

		FString TimeText = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
		BlasterHUD->CharacterOverlay->MatchTimerText->SetText(FText::FromString(TimeText));
	}
}

void ABlasterPlayerController::SetMatchTime()
{
	uint32 SecondsLeft = FMath::CeilToInt(MatchTime - GetServerTime());
	if (SecondsLeft != CountDownInt)
		DisplayMatchTimer(MatchTime - GetServerTime());

	CountDownInt = SecondsLeft;
}

void ABlasterPlayerController::ServerRequestServerTime_Implementation(float TimeOfClientRequest)
{
	ClientReportServerTime(TimeOfClientRequest, GetWorld()->GetTimeSeconds());
}

void ABlasterPlayerController::ClientReportServerTime_Implementation(float TimeOfClientRequest, float TimeServerRecievedClientRequest)
{
	float RoundTripTime = GetWorld()->GetTimeSeconds() - TimeOfClientRequest;
	float CurrentServerTime = TimeServerRecievedClientRequest + (RoundTripTime / 2.f);
	ServerDelta = CurrentServerTime - GetWorld()->GetTimeSeconds();
}

float ABlasterPlayerController::GetServerTime()
{
	if (HasAuthority())
		return GetWorld()->GetTimeSeconds();
	else
		return GetWorld()->GetTimeSeconds() + ServerDelta;
}