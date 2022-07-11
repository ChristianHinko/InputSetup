// Fill out your copyright notice in the Description page of Project Settings.


#include "ISDeveloperSettings_InputSetup.h"

#include "InputMappingContext.h"



UISDeveloperSettings_InputSetup::UISDeveloperSettings_InputSetup()
{
	CategoryName = "Plugins";
}


const UInputAction* UISDeveloperSettings_InputSetup::GetInputActionByTag(const FGameplayTag& InTag) const
{
	if (const TSoftObjectPtr<const UInputAction>* InputActionFound = InputActionTagMap.Find(InTag))
	{
		return InputActionFound->LoadSynchronous();
	}

	return nullptr;
}
