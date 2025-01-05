// Fill out your copyright notice in the Description page of Project Settings.

#include "ActorComponents/ISActorComponent_PawnExtension.h"

#include "GCUtils.h"
#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "GCUtils_Log.h"
#include "GCUtils_String.h"

DEFINE_LOG_CATEGORY_STATIC(LogISActorComponent_PawnExtension, Log, All);

UISActorComponent_PawnExtension::UISActorComponent_PawnExtension(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UISActorComponent_PawnExtension::OnOwnerPawnClientRestart()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UISActorComponent_PawnExtension::OnOwnerPawnClientRestart);

    GC_LOG_STR_UOBJECT(
        this,
        LogISActorComponent_PawnExtension,
        Log,
        TEXT("On owner pawn client restart. Attempting to add input mapping contexts."));

    check(IsValid(GetOwner()));
    const APawn& owningPawn = GCUtils::StaticCastChecked<APawn&>(GetOwner());

    const APlayerController* playerController = Cast<APlayerController>(owningPawn.GetController());
    if (!ensure(playerController))
    {
        return;
    }

    const ULocalPlayer* localPlayer = playerController->GetLocalPlayer();
    if (!ensure(localPlayer))
    {
        return;
    }

    UEnhancedInputLocalPlayerSubsystem* enhancedInputLocalPlayerSubsystem = localPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
    if (!ensure(enhancedInputLocalPlayerSubsystem))
    {
        return;
    }

    // Clear mappings. TODO @techdebt: It doesn't make sense to clear ALL of the player's input mappings
    // here. Any previously-added mappings should ideally be cleared in a specific manner, instead of a player-wide
    // wipe of all mappings.
    enhancedInputLocalPlayerSubsystem->ClearAllMappings();

    // Add new input mapping contexts.
    for (const FISInputMappingContextAddArgs& inputMappingContextAddArgs : InputMappingContextsToAdd)
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogISActorComponent_PawnExtension,
            Verbose,
            WriteToString<256>(
                TEXT("Adding input mapping context '"),
                GCUtils::String::GetUObjectPathNameSafe(inputMappingContextAddArgs.InputMappingContext),
                TEXT("'. Priority: `"),
                inputMappingContextAddArgs.Priority,
                TEXT("`.")
                )
            );

        enhancedInputLocalPlayerSubsystem->AddMappingContext(
            inputMappingContextAddArgs.InputMappingContext,
            inputMappingContextAddArgs.Priority,
            inputMappingContextAddArgs.ModifyContextOptions);
    }
}
