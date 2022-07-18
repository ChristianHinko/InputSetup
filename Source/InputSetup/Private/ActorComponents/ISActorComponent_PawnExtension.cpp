// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorComponents/ISActorComponent_PawnExtension.h"

#include "GameFramework/Pawn.h"
#include "EnhancedInputComponent.h"
#include "PlayerMappableInputConfig.h"
#include "EnhancedInputSubsystems.h"



UISActorComponent_PawnExtension::UISActorComponent_PawnExtension(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}


void UISActorComponent_PawnExtension::PawnClientRestart()
{
	// Clear and add input configs since pawn restarting
	const APawn* OwningPawn = Cast<APawn>(GetOwner());
	if (IsValid(OwningPawn))
	{
		const APlayerController* PlayerController = Cast<APlayerController>(OwningPawn->GetController());
		if (IsValid(PlayerController))
		{
			const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
			if (IsValid(LocalPlayer))
			{
				UEnhancedInputLocalPlayerSubsystem* EnhancedInputLocalPlayerSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
				if (IsValid(EnhancedInputLocalPlayerSubsystem))
				{
					// Clear mappings
					EnhancedInputLocalPlayerSubsystem->ClearAllMappings();

					// Add default configs
					for (UPlayerMappableInputConfig* PlayerMappableInputConfig : DefaultPlayerMappableInputConfigs)
					{
						EnhancedInputLocalPlayerSubsystem->AddPlayerMappableConfig(PlayerMappableInputConfig);
					}
				}
			}
		}
	}
}

void UISActorComponent_PawnExtension::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{

}
