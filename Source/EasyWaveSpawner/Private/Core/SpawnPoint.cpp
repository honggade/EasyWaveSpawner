#include "Core/SpawnPoint.h"

#include "Components/BillboardComponent.h"


ASpawnPoint::ASpawnPoint()
{
	PrimaryActorTick.bCanEverTick = false;
	USceneComponent* Root = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = Root;
#if WITH_EDITOR
	// 只有在编辑器模式下才创建这个组件
	SpriteComponent = CreateDefaultSubobject<UBillboardComponent>(TEXT("Sprite"));
	if (SpriteComponent)
	{
		SpriteComponent->SetupAttachment(RootComponent);
        
		static ConstructorHelpers::FObjectFinder<UTexture2D> DefaultTexture(TEXT("/Engine/EditorResources/S_Pawn"));
		if (DefaultTexture.Succeeded())
		{
			SpriteComponent->SetSprite(DefaultTexture.Object);
		}
	}
#endif

}



void ASpawnPoint::BeginPlay()
{
	Super::BeginPlay();
	
}

void ASpawnPoint::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

