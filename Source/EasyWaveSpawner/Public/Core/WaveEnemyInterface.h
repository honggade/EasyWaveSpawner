#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "WaveEnemyInterface.generated.h"

UINTERFACE()
class UWaveEnemyInterface : public UInterface
{
	GENERATED_BODY()
};

class EASYWAVESPAWNER_API IWaveEnemyInterface
{
	GENERATED_BODY()

public:
	// 诉怪物它属于哪个波次管理器 这样怪物死的时候就知道去哪儿报丧
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Wave")
	void AssignWaveManager(class UWaveManagerComponent* Manager);

	//	这是一个标记函数，用来判断怪物是否准备好被清理
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Wave")
	bool IsWaveRelevant() const;
};
