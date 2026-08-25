#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "MyAIController.generated.h"

UCLASS()
class MYPROJECT_API AMyAIController : public AAIController
{
	GENERATED_BODY()

public:
	AMyAIController();

protected:
	virtual void BeginPlay() override;

	// Указатель на наше Дерево Поведения
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI")
	class UBehaviorTree* BehaviorTreeAsset;
};
