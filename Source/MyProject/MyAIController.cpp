#include "MyAIController.h"
#include "BehaviorTree/BehaviorTree.h"

AMyAIController::AMyAIController()
{
	// Контроллер будет обрабатывать логику ИИ
	PrimaryActorTick.bCanEverTick = false;
}

void AMyAIController::BeginPlay()
{
	Super::BeginPlay();

	// Запускаем дерево поведения, если оно задано
	if (BehaviorTreeAsset)
	{
		RunBehaviorTree(BehaviorTreeAsset);
		UE_LOG(LogTemp, Log, TEXT("AI Controller: Behavior Tree started successfully."));
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("AI Controller: BehaviorTreeAsset is missing!"));
	}
}
