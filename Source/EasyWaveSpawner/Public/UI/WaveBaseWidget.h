#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Data/WaveTypes.h"
#include "WaveBaseWidget.generated.h"

UCLASS()
class EASYWAVESPAWNER_API UWaveBaseWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:
	
	UPROPERTY(BlueprintReadOnly, Category = "Wave")
	class UWaveManagerComponent* AssociatedManager;
	

	
	// 这是一个可以在蓝图中实现的事件，当波次状态改变时被调用
	UFUNCTION(BlueprintImplementableEvent, Category = "Wave")
	void OnWaveStatusUpdated(EWaveState NewState);

	UFUNCTION(BlueprintImplementableEvent, Category = "Wave")
	void OnCountdownTicked(float RemainingTime);

	UFUNCTION(BlueprintImplementableEvent, Category = "Wave")
	void OnProgressUpdated(int32 RemainingEnemies, int32 TotalEnemies);
};
