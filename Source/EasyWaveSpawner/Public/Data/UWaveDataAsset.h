// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WaveTypes.h"
#include "Engine/DataAsset.h"
#include "UWaveDataAsset.generated.h"


UCLASS(BlueprintType)
class EASYWAVESPAWNER_API UUWaveDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Waves")
	TArray<FWaveDefinition> WaveSequence; // 关卡的波次列表

	// 辅助函数：根据索引获取波次数据
	UFUNCTION(BlueprintPure, Category = "Waves")
	bool GetWaveDefinition(int32 Index, FWaveDefinition& OutDefinition) const
	{
		if (WaveSequence.IsValidIndex(Index))
		{
			OutDefinition = WaveSequence[Index];
			UE_LOG(LogTemp, Log, TEXT("传入波次index有效"));
			return true;
		}
		UE_LOG(LogTemp, Log, TEXT("传入波次index无效"));
		return false;
	}
};
