// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"

#include "ISPrimaryDataAsset_InputActionAssetReferences.generated.h"

struct FGameplayTag;
class UInputAction;

/**
 * @brief Plugins can use this data asset to contribute to the game's asset references. This asset
 *        must be used as "Input/IAAR_<plugin-name>" where "<plugin-name>" is the name of your plugin.
 */
UCLASS(Const)
class INPUTSETUP_API UISPrimaryDataAsset_InputActionAssetReferences : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:

    UPROPERTY(EditDefaultsOnly)
    TMap<FGameplayTag, TObjectPtr<UInputAction>> InputActionReferences;
};
