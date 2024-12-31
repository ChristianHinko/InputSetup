// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"

/**
 * @brief This module's native gameplay tags wrapped in a namespace.
 */
namespace ISNativeGameplayTags
{
    INPUTSETUP_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputAction);
    INPUTSETUP_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputAction_None);
}
