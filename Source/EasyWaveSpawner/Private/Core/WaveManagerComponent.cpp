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
    if (CurrentState != EWaveState::Idle && CurrentState != EWaveState::WaveComplete){ return; }
    if (WaveDataAsset && WaveDataAsset->WaveSequence.Num() > 0)
    {
        if (CurrentWaveIndex == -1 ){ CurrentWaveIndex = 0; }
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
    OnCountdownUpdated.Broadcast(CurrentCountdownTime, 0.0f);

    if (CurrentCountdownTime <= 0)
    {
        GetWorld()->GetTimerManager().ClearTimer(TimerHandle_StateDelay);
        StartSpawning();
    }
}

void UWaveManagerComponent::StartSpawning()
{
    if (!WaveDataAsset) return;
    
    FWaveDefinition CurrentWave;
    if (!WaveDataAsset->GetWaveDefinition(CurrentWaveIndex, CurrentWave)) return;

    SetState(EWaveState::Spawning);
    
    // 1. 重置并构建怪物大名单
    TotalEnemiesInCurrentWave = 0;
    CurrentWaveEnemyQueue.Empty();

    for (const FWaveEnemyEntry& EnemyData : CurrentWave.Enemies)
    {
        TotalEnemiesInCurrentWave += EnemyData.Count;
        for (int32 i = 0; i < EnemyData.Count; i++)
        {
            CurrentWaveEnemyQueue.Add(EnemyData.EnemyClass);
        }
    }

    // 2. 【新增功能】根据开关决定是否打乱怪物顺序
    if (bShuffleEnemyQueue && CurrentWaveEnemyQueue.Num() > 0)
    {
        for (int32 i = CurrentWaveEnemyQueue.Num() - 1; i > 0; i--)
        {
            int32 j = FMath::RandRange(0, i);
            CurrentWaveEnemyQueue.Swap(i, j);
        }
    }

    // 3. 初始化其它变量
    PendingEnemiesToSpawn = TotalEnemiesInCurrentWave;
    ActiveEnemyCount = 0;
    NextEnemyQueueIndex = 0;

    // 4. 初始化洗牌后的生成点（这一步建议始终保持洗牌，以保证位置均匀）
    ShuffledSpawnPoints = CachedSpawnPoints;
    if (ShuffledSpawnPoints.Num() > 0)
    {
        for (int32 i = ShuffledSpawnPoints.Num() - 1; i > 0; i--)
        {
            int32 j = FMath::RandRange(0, i);
            ShuffledSpawnPoints.Swap(i, j);
        }
    }
    NextSpawnPointIndex = 0;

    // 5. 开启定时器
    float Interval = CurrentWave.Enemies.Num() > 0 ? CurrentWave.Enemies[0].SpawnInterval : 1.0f;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle_StateDelay, this, &UWaveManagerComponent::SpawnSingleEnemy, Interval, true);
}

void UWaveManagerComponent::SpawnSingleEnemy()
{
    // 如果名单为空或已经刷完，停止
    if (PendingEnemiesToSpawn <= 0 || !CurrentWaveEnemyQueue.IsValidIndex(NextEnemyQueueIndex))
    {
        GetWorld()->GetTimerManager().ClearTimer(TimerHandle_StateDelay);
        SetState(EWaveState::InProgress);
        return;
    }

    // 1. 从大名单按顺序取怪（名单本身可能已经洗过牌了）
    TSubclassOf<AActor> EnemyClass = CurrentWaveEnemyQueue[NextEnemyQueueIndex];
    NextEnemyQueueIndex++;

    // 2. 从洗牌后的生成点队列取点
    if (ShuffledSpawnPoints.Num() == 0) return;
    AActor* SelectedPoint = ShuffledSpawnPoints[NextSpawnPointIndex];
    NextSpawnPointIndex = (NextSpawnPointIndex + 1) % ShuffledSpawnPoints.Num();

    // 3. 生成逻辑
    FActorSpawnParameters SpawnParams;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    AActor* NewEnemy = GetWorld()->SpawnActor<AActor>(EnemyClass, SelectedPoint->GetActorTransform(), SpawnParams);

    if (NewEnemy)
    {
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
        // 判断是否自动开始下一波
        if (bAutoStartNextWave)
        {
            // 自动开始：延迟几秒后进入准备阶段
            FTimerHandle NextWaveHandle;
            GetWorld()->GetTimerManager().SetTimer(NextWaveHandle, this, &UWaveManagerComponent::StartPreparation, 2.0f, false);
        }
        else
        {
            // 手动开始：系统进入 Idle 状态，不再有动作，直到开发者再次调用 StartWaveSystem()
            // 你也可以在这里广播一个自定义委托，告诉 UI 显示“按G开始下一波”
            SetState(EWaveState::Idle); 
        }
    }
    else
    {
        SetState(EWaveState::AllCompleted);
    }
}

void UWaveManagerComponent::GetWavePreviewInfo(int32 WaveIndex, int32& OutTotalEnemies, float& OutPrepTime)
{
    OutTotalEnemies = 0;
    OutPrepTime = 0.0f;

    if (!WaveDataAsset) return;

    // 逻辑：如果游戏还没开始（Index为-1），我们默认给 UI 展示第 0 波（第一波）的数据
    int32 TargetIndex = (WaveIndex < 0) ? 0 : WaveIndex;

    FWaveDefinition WaveDef;
    if (WaveDataAsset->GetWaveDefinition(TargetIndex, WaveDef))
    {
        // 关键一步：给引用参数赋值，蓝图引脚就会输出这些值
        OutPrepTime = WaveDef.PreparationTime;
        
        int32 CountSum = 0;
        for (const auto& EnemyEntry : WaveDef.Enemies)
        {
            CountSum += EnemyEntry.Count;
        }
        OutTotalEnemies = CountSum;
    }
}

void UWaveManagerComponent::SetState(EWaveState NewState)
{
    CurrentState = NewState;
    OnWaveStateChanged.Broadcast(NewState);
}
