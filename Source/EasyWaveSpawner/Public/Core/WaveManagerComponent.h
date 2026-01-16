// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Data/WaveTypes.h"
#include "WaveManagerComponent.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class EASYWAVESPAWNER_API UWaveManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UWaveManagerComponent();

protected:
	virtual void BeginPlay() override;

public:
	/** 开始波次系统（手动调用或 BeginPlay 自动调用） */
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void StartWaveSystem();

	/** 当怪物死亡时，由怪物或血量组件调用 */
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void ReportEnemyDeath(AActor* DeadEnemy);

	// --- 委托 (UI 绑定) ---
	UPROPERTY(BlueprintAssignable, Category = "Wave|Events")
	FOnWaveStateChanged OnWaveStateChanged;

	UPROPERTY(BlueprintAssignable, Category = "Wave|Events")
	FOnCountdownUpdated OnCountdownUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Wave|Events")
	FOnWaveProgressChanged OnWaveProgressChanged;

protected:
	/** 波次配置文件 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave|Config")
	class UUWaveDataAsset* WaveDataAsset;

private:
	// --- 内部状态变量 ---
	int32 CurrentWaveIndex = -1;
	int32 PendingEnemiesToSpawn = 0; // 这一波还剩多少怪没刷出来
	int32 ActiveEnemyCount = 0;      // 场上还剩多少活着的怪
	int32 TotalEnemiesInCurrentWave = 0;
    
	EWaveState CurrentState = EWaveState::Idle;
	FTimerHandle TimerHandle_StateDelay; // 用于处理倒计时和刷怪间隔
    
	// 缓存场景中的生成点
	UPROPERTY()
	TArray<AActor*> CachedSpawnPoints;

	// --- 内部逻辑函数 ---
	void SetState(EWaveState NewState);
	void StartPreparation();
	void StartSpawning();
	void SpawnSingleEnemy();
	void CheckWaveProgress();
	void UpdateCountdown();
    
	float CurrentCountdownTime = 0.0f;
};
