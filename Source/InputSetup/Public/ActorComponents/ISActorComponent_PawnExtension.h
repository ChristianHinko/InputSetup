// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"

#include "ISActorComponent_PawnExtension.generated.h"


class UPlayerMappableInputConfig;



/**
 * Sets up input for Pawns.
 * 
 * ----------------------------------
 *				Setup
 * ----------------------------------
 * 
 * 
 */
UCLASS(ClassGroup=(InputSetup), meta=(BlueprintSpawnableComponent), Within=Pawn)
class INPUTSETUP_API UISActorComponent_PawnExtension : public UActorComponent
{
	GENERATED_BODY()

public:
	UISActorComponent_PawnExtension(const FObjectInitializer& ObjectInitializer);


	UPROPERTY(EditAnywhere, Category = "Input|Config")
		TArray<TObjectPtr<UPlayerMappableInputConfig>> DefaultPlayerMappableInputConfigs;

	/** Called at the end of your Pawn's PawnClientRestart() event */
	void PawnClientRestart();
	/** Called at the end of your Pawn's SetupPlayerInputComponent() event */
	void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent);

protected:

};
