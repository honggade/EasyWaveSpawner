#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WaveTypes.generated.h"

UCLASS()
class EASYWAVESPAWNER_API UWaveTypes : public UObject
{
	GENERATED_BODY()
};

UENUM(BlueprintType)
enum class EWaveState : uint8
{
	Idle          UMETA(DisplayName = "闲置"),
	Preparing     UMETA(DisplayName = "中场休息/倒计时"),
	Spawning      UMETA(DisplayName = "生成怪物中"),
	InProgress    UMETA(DisplayName = "战斗进行中"),
	WaveComplete  UMETA(DisplayName = "本波结束"),
	AllCompleted  UMETA(DisplayName = "关卡全满完成")
};

// 定义单种怪物的生成信息
USTRUCT(BlueprintType)
struct FWaveEnemyEntry
{
	GENERATED_BODY()

	// 生成哪个怪
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TSubclassOf<AActor> EnemyClass; 
	
	// 这一类生成多少个
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	int32 Count = 5; 

	// 每一个怪生成的间隔（秒）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float SpawnInterval = 1.0f; 
};

// 定义一个完整波次的内容
USTRUCT(BlueprintType)
struct FWaveDefinition
{
	GENERATED_BODY()

	// 这一波里有哪些怪物组合
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	TArray<FWaveEnemyEntry> Enemies; 

	// 这一波开始前的倒计时（中场休息时间）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	float PreparationTime = 5.0f; 

	// UI显示的名称
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Wave")
	FString WaveTitle = TEXT("新波次");
};

// 定义 UI 通信用的委托（多播动态委托，方便蓝图绑定）
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWaveStateChanged, EWaveState, NewState);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnCountdownUpdated, float, TimeRemaining, float, TotalTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWaveProgressChanged, int32, EnemiesRemaining, int32, TotalEnemies);
