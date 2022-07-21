// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "ISPrimaryDataAsset_PluginInputActions.generated.h"


struct FGameplayTag;
class UInputAction;



/**
 * Plugins can use this Data Asset to contribute to the game's Input Actions.
 * This asset is required to be in the Content directory as: "/Input/PDA_InputActions.uasset".
 */
UCLASS()
class INPUTSETUP_API UISPrimaryDataAsset_PluginInputActions : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly)
		TMap<FGameplayTag, TObjectPtr<const UInputAction>> InputActionsToAdd;
};
