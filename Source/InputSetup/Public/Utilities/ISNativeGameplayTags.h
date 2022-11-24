// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NativeGameplayTags.h"



/**
 * This module's Native Gameplay Tags wrapped in a namespace.
 */
namespace ISNativeGameplayTags
{
	INPUTSETUP_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputAction) // parent tag of all InputAction tags
	INPUTSETUP_API UE_DECLARE_GAMEPLAY_TAG_EXTERN(InputAction_None) // InputAction tag to use when you want to specify that there is no Input Action
}
