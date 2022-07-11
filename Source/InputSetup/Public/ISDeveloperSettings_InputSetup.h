// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DeveloperSettings.h"

#include "ISDeveloperSettings_InputSetup.generated.h"


class UInputAction;



/**
 * 
 */
UCLASS(config=Game, defaultconfig, meta = (DisplayName = "Input Setup"))
class INPUTSETUP_API UISDeveloperSettings_InputSetup : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UISDeveloperSettings_InputSetup();


	/** Input Actions mapped to Gameplay Tags. Used to identify an Input Action with a Gameplay Tag. */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, config)
		TMap<FGameplayTag, TSoftObjectPtr<const UInputAction>> InputActionTagMap;


	const UInputAction* GetInputActionByTag(const FGameplayTag& InTag) const;
};
