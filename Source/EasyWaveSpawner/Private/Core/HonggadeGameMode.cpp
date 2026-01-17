// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/HonggadeGameMode.h"

AHonggadeGameMode::AHonggadeGameMode()
{
	WaveManager = CreateDefaultSubobject<UWaveManagerComponent>(TEXT("WaveManagerComp"));
}
