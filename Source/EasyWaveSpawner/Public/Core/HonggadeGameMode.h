// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WaveManagerComponent.h"
#include "GameFramework/GameModeBase.h"
#include "HonggadeGameMode.generated.h"

/**
 * 
 */
UCLASS()
class EASYWAVESPAWNER_API AHonggadeGameMode : public AGameModeBase
{
	GENERATED_BODY()
public:
	AHonggadeGameMode();
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wave")
	UWaveManagerComponent* WaveManager;
};
