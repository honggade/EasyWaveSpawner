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

	// 是否自动开始下一波。如果为 false，每一波结束后系统会进入 Idle，等待手动再次调用 StartWaveSystem 或自定义函数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave|Config")
	bool bAutoStartNextWave = true;
	
	// 当前波次
	UPROPERTY(BlueprintReadOnly, Category = "Wave|Status")
    int32 CurrentWaveIndex = -1;
	
    // 这一波还剩多少怪没刷出来
	UPROPERTY(BlueprintReadOnly, Category = "Wave|Status")
    int32 PendingEnemiesToSpawn = 0;
    
    // 场上还剩多少活着的怪
	UPROPERTY(BlueprintReadOnly, Category = "Wave|Status")
    int32 ActiveEnemyCount = 0;      
    
    // 当前波次总怪物量
	UPROPERTY(BlueprintReadOnly, Category = "Wave|Status")
    int32 TotalEnemiesInCurrentWave = 0;
       
    // 当前状态
	UPROPERTY(BlueprintReadOnly, Category = "Wave|Status")
    EWaveState CurrentState = EWaveState::Idle;
    
    // 用于处理倒计时和刷怪间隔
	UPROPERTY(BlueprintReadOnly, Category = "Wave|Status")
    FTimerHandle TimerHandle_StateDelay; 

	// 倒计时剩余时间
	UPROPERTY(BlueprintReadOnly, Category = "Wave|Status")
	float CurrentCountdownTime = 0.0f;
	
	/** 供UI调用：获取指定波次的预览信息（总人数、准备时间） */
	UFUNCTION(BlueprintCallable, Category = "Wave")
	void GetWavePreviewInfo(int32 WaveIndex, int32& OutTotalEnemies, float& OutPrepTime);
	
protected:
	/** 波次配置文件 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave|Config")
	class UUWaveDataAsset* WaveDataAsset;

private:
	// 缓存场景中的生成点
	UPROPERTY()
	TArray<AActor*> CachedSpawnPoints;
	
	// 专门用于本波次循环使用的打乱后的生成点队列
	UPROPERTY()
	TArray<AActor*> ShuffledSpawnPoints;

	// 当前取到了第几个点
	int32 NextSpawnPointIndex = 0;

	// --- 内部逻辑函数 ---
	void SetState(EWaveState NewState);
	void StartPreparation();
	void StartSpawning();
	void SpawnSingleEnemy();
	void CheckWaveProgress();
	void UpdateCountdown();
    
	
};
