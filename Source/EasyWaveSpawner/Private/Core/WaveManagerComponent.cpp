// Fill out your copyright notice in the Description page of Project Settings.


#include "Core/WaveManagerComponent.h"

#include "Core/SpawnPoint.h"
#include "Core/WaveEnemyInterface.h"
#include "Data/UWaveDataAsset.h"
#include "Kismet/GameplayStatics.h"


// Sets default values for this component's properties
UWaveManagerComponent::UWaveManagerComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}
void UWaveManagerComponent::BeginPlay()
{
	Super::BeginPlay();
    UGameplayStatics::GetAllActorsOfClass(GetWorld(),ASpawnPoint::StaticClass(),CachedSpawnPoints);
    
}

void UWaveManagerComponent::StartWaveSystem()
{
    if (WaveDataAsset && WaveDataAsset->WaveSequence.Num() > 0)
    {
        CurrentWaveIndex = 0;
        StartPreparation();
    }
}

void UWaveManagerComponent::StartPreparation()
{
    SetState(EWaveState::Preparing);
    
    FWaveDefinition CurrentWave;
    if (WaveDataAsset->GetWaveDefinition(CurrentWaveIndex, CurrentWave))
    {
        CurrentCountdownTime = CurrentWave.PreparationTime;
        // 开启每秒执行一次的倒计时计时器
        GetWorld()->GetTimerManager().SetTimer(TimerHandle_StateDelay, this, &UWaveManagerComponent::UpdateCountdown, 1.0f, true);
    }
}

void UWaveManagerComponent::UpdateCountdown()
{
    CurrentCountdownTime -= 1.0f;
    OnCountdownUpdated.Broadcast(CurrentCountdownTime, 0.0f); // 0.0f 处可以传入总时间

    if (CurrentCountdownTime <= 0)
    {
        GetWorld()->GetTimerManager().ClearTimer(TimerHandle_StateDelay);
        StartSpawning();
    }
}

void UWaveManagerComponent::StartSpawning()
{
    SetState(EWaveState::Spawning);
    
    FWaveDefinition CurrentWave;
    WaveDataAsset->GetWaveDefinition(CurrentWaveIndex, CurrentWave);

    // 计算总怪数
    TotalEnemiesInCurrentWave = 0;
    for (const auto& Entry : CurrentWave.Enemies) { TotalEnemiesInCurrentWave += Entry.Count; }
    
    PendingEnemiesToSpawn = TotalEnemiesInCurrentWave;
    ActiveEnemyCount = 0;

    // 开始周期性刷怪
    float Interval = CurrentWave.Enemies.Num() > 0 ? CurrentWave.Enemies[0].SpawnInterval : 1.0f;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle_StateDelay, this, &UWaveManagerComponent::SpawnSingleEnemy, Interval, true);
}

void UWaveManagerComponent::SpawnSingleEnemy()
{
    if (PendingEnemiesToSpawn <= 0 || CachedSpawnPoints.Num() == 0)
    {
        GetWorld()->GetTimerManager().ClearTimer(TimerHandle_StateDelay);
        SetState(EWaveState::InProgress);
        return;
    }

    // 1. 找到当前该刷哪种怪（这里简化逻辑：按顺序刷）
    FWaveDefinition WaveDef;
    WaveDataAsset->GetWaveDefinition(CurrentWaveIndex, WaveDef);
    
    // 2. 随机选个生成点
    AActor* SelectedPoint = CachedSpawnPoints[FMath::RandRange(0, CachedSpawnPoints.Num() - 1)];
    
    // 3. 生成怪物
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    TSubclassOf<AActor> EnemyClass = WaveDef.Enemies[0].EnemyClass; // 实际开发可做更复杂的权重选择
    AActor* NewEnemy = GetWorld()->SpawnActor<AActor>(EnemyClass, SelectedPoint->GetActorTransform(), SpawnParams);

    if (NewEnemy)
    {
        // 如果怪物实现了接口，给它分配管理器
        if (NewEnemy->GetClass()->ImplementsInterface(UWaveEnemyInterface::StaticClass()))
        {
            IWaveEnemyInterface::Execute_AssignWaveManager(NewEnemy, this);
        }
        
        ActiveEnemyCount++;
        PendingEnemiesToSpawn--;
        OnWaveProgressChanged.Broadcast(ActiveEnemyCount, TotalEnemiesInCurrentWave);
    }
}

void UWaveManagerComponent::ReportEnemyDeath(AActor* DeadEnemy)
{
    ActiveEnemyCount--;
    OnWaveProgressChanged.Broadcast(ActiveEnemyCount, TotalEnemiesInCurrentWave);
    
    if (ActiveEnemyCount <= 0 && PendingEnemiesToSpawn <= 0)
    {
        CheckWaveProgress();
    }
}

void UWaveManagerComponent::CheckWaveProgress()
{
    CurrentWaveIndex++;
    if (CurrentWaveIndex < WaveDataAsset->WaveSequence.Num())
    {
        SetState(EWaveState::WaveComplete);
        // 延迟一会儿进入下一波
        FTimerHandle NextWaveHandle;
        GetWorld()->GetTimerManager().SetTimer(NextWaveHandle, this, &UWaveManagerComponent::StartPreparation, 3.0f, false);
    }
    else
    {
        SetState(EWaveState::AllCompleted);
    }
}

void UWaveManagerComponent::SetState(EWaveState NewState)
{
    CurrentState = NewState;
    OnWaveStateChanged.Broadcast(NewState);
}