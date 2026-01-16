#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SpawnPoint.generated.h"

UCLASS()
class EASYWAVESPAWNER_API ASpawnPoint : public AActor
{
	GENERATED_BODY()

public:
	ASpawnPoint();

	// 可以给生成点加标签，比如 "BossOnly" 或 "SideDoor"
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawn")
	FName SpawnTag;
	
	// 仅保留一个默认组件，编辑器里能看到个图标就行
	UPROPERTY()
	class UBillboardComponent* SpriteComponent;
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
