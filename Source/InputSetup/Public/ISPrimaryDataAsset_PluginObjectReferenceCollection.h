// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "ISPrimaryDataAsset_PluginObjectReferenceCollection.generated.h"


struct FGameplayTag;
class UInputAction;



/**
 * Plugins can use this data asset to contribute to the game's object references.
 * This asset must be used as "Input/PORC_PluginName" where "PluginName" is the name of your plugin.
 */
UCLASS(Const)
class INPUTSETUP_API UISPrimaryDataAsset_PluginObjectReferenceCollection : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly)
        TMap<FGameplayTag, TObjectPtr<UInputAction>> PluginInputActionReferences;
};
