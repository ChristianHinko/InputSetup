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

    /**
     * TODO @techdebt: UPlayerMappableInputConfig is deprecated as of 5.3.
     *
     * TODO @techdebt: We need to research/investigate the best way to give our InputMappingContexts. It doesn't feel like UPlayerMappableInputConfig
     * is how our game's UInputMappingContext should be given since input mapping feels like a separate thing. As of 5.3 there is now a
     * UEnhancedInputDeveloperSettings::DefaultMappingContexts, but we tried using it and no trigger events occur. It seems like a bug.
     *
     * UDN post: https://udn.unrealengine.com/s/question/0D54z00009nwTNVCA2/enhanced-input-default-context-mappings-clearing-out-custom-triggers
     * Jira issue: https://issues.unrealengine.com/issue/UE-199784
     */
    UPROPERTY(EditAnywhere, Category = "Input|Config")
    TArray<TObjectPtr<UPlayerMappableInputConfig>> PlayerMappableInputConfigsToAdd;

    /** Call at the end of your Pawn's PawnClientRestart() event */
    void OnOwnerPawnClientRestart();
};
