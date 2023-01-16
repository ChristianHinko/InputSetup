// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "ISActorComponent_PawnExtension.generated.h"


class UPlayerMappableInputConfig;
struct FGameplayTag;



/**
 * Sets up input for Pawns.
 */
UCLASS(ClassGroup=(InputSetup), meta=(BlueprintSpawnableComponent), Within=Pawn)
class INPUTSETUP_API UISActorComponent_PawnExtension : public UActorComponent
{
	GENERATED_BODY()

public:
	UISActorComponent_PawnExtension(const FObjectInitializer& ObjectInitializer);


	UPROPERTY(EditAnywhere, Category = "Input|Config")
		TArray<TObjectPtr<UPlayerMappableInputConfig>> DefaultPlayerMappableInputConfigs;

	/** Call at the end of your Pawn's PawnClientRestart() event */
	void OnOwnerPawnClientRestart();
};
