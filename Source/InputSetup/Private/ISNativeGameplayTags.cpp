// Fill out your copyright notice in the Description page of Project Settings.

#include "ISNativeGameplayTags.h"

namespace ISNativeGameplayTags
{
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputAction, TEXT("InputAction"), "The parent tag for all input action tags.");
    UE_DEFINE_GAMEPLAY_TAG_COMMENT(InputAction_None, TEXT("InputAction.None"), "The tag to use when you want to explicitly specify the absence of an input action.");
}
