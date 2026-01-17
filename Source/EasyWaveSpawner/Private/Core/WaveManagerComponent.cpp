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
    if (!WaveDataAsset->GetWaveDefinition(CurrentWaveIndex, CurrentWave)) return;

    // 1. 初始化统计数据
    TotalEnemiesInCurrentWave = 0;
    for (const auto& Entry : CurrentWave.Enemies) { TotalEnemiesInCurrentWave += Entry.Count; }
    PendingEnemiesToSpawn = TotalEnemiesInCurrentWave;
    ActiveEnemyCount = 0;

    // 2. 初始化洗牌队列
    ShuffledSpawnPoints = CachedSpawnPoints; // 复制一份
    if (ShuffledSpawnPoints.Num() > 0)
    {
        // 使用内置算法打乱顺序
        for (int32 i = ShuffledSpawnPoints.Num() - 1; i > 0; i--)
        {
            int32 j = FMath::RandRange(0, i);
            ShuffledSpawnPoints.Swap(i, j);
        }
    }
    NextSpawnPointIndex = 0; // 重置索引

    // 3. 开始周期性刷怪
    float Interval = CurrentWave.Enemies.Num() > 0 ? CurrentWave.Enemies[0].SpawnInterval : 1.0f;
    GetWorld()->GetTimerManager().SetTimer(TimerHandle_StateDelay, this, &UWaveManagerComponent::SpawnSingleEnemy, Interval, true);
}

void UWaveManagerComponent::SpawnSingleEnemy()
{
    // 如果没怪了或没点，停止计时器
    if (PendingEnemiesToSpawn <= 0 || ShuffledSpawnPoints.Num() == 0)
    {
        GetWorld()->GetTimerManager().ClearTimer(TimerHandle_StateDelay);
        SetState(EWaveState::InProgress);
        return;
    }

    // 1. 按照洗牌后的顺序选择生成点
    AActor* SelectedPoint = ShuffledSpawnPoints[NextSpawnPointIndex];
    
    // 递增索引，如果用完了所有点，重新洗牌开启下一轮循环
    NextSpawnPointIndex++;
    if (NextSpawnPointIndex >= ShuffledSpawnPoints.Num())
    {
        NextSpawnPointIndex = 0;
        // 再次打乱以防下一轮循环路径死板
        for (int32 i = ShuffledSpawnPoints.Num() - 1; i > 0; i--)
        {
            int32 j = FMath::RandRange(0, i);
            ShuffledSpawnPoints.Swap(i, j);
        }
    }

    // 2. 准备怪物类别
    FWaveDefinition WaveDef;
    WaveDataAsset->GetWaveDefinition(CurrentWaveIndex, WaveDef);
    TSubclassOf<AActor> EnemyClass = WaveDef.Enemies[0].EnemyClass;

    // 3. 执行生成
    FActorSpawnParameters SpawnParams;
    // 强制生成，哪怕有碰撞也会尝试挤开或重叠，确保怪物数量不会少
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

void UWaveManagerComponent::SetState(EWaveState NewState)
{
    CurrentState = NewState;
    OnWaveStateChanged.Broadcast(NewState);
}