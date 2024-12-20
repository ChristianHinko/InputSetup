// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputSubsystemInterface.h"

#include "ISInputMappingContextAddArgs.generated.h"

class UInputMappingContext;

/**
 * @brief All arguments to pass into the function `IEnhancedInputSubsystemInterface::AddMappingContext()`.
 */
USTRUCT(BlueprintType)
struct INPUTSETUP_API FISInputMappingContextAddArgs
{
    GENERATED_BODY()

public:

    UPROPERTY(EditAnywhere)
    TObjectPtr<const UInputMappingContext> InputMappingContext = nullptr;

    UPROPERTY(EditAnywhere)
    int32 Priority = 0;

    UPROPERTY(EditAnywhere)
    FModifyContextOptions ModifyContextOptions;
};
