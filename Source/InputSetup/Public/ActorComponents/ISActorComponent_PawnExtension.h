// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/ISInputMappingContextAddArgs.h"

#include "ISActorComponent_PawnExtension.generated.h"

INPUTSETUP_API DECLARE_LOG_CATEGORY_EXTERN(LogISActorComponent_PawnExtension, Log, All);

/**
 * @brief Sets up input for Pawns.
 */
UCLASS(ClassGroup="InputSetup", Within="Pawn", meta=(BlueprintSpawnableComponent))
class INPUTSETUP_API UISActorComponent_PawnExtension : public UActorComponent
{
    GENERATED_BODY()

public:

    UISActorComponent_PawnExtension(const FObjectInitializer& ObjectInitializer);

public:

    /**
     * @brief Call at the end of your Pawn's PawnClientRestart() event.
     */
    void OnOwnerPawnClientRestart();

public:

    /**
     * @brief Collection of input mapping contexts to add for the owner pawn.
     * @todo @techdebt Eliminate hard asset reference here. Make a soft version of this struct and
     *       perform proper asset loading.
     */
    UPROPERTY(EditAnywhere, Category = "InputSetup")
    TArray<FISInputMappingContextAddArgs> InputMappingContextsToAdd;
};
