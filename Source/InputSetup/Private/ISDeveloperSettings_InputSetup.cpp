// Fill out your copyright notice in the Description page of Project Settings.


#include "ISDeveloperSettings_InputSetup.h"

#include "InputAction.h"



UISDeveloperSettings_InputSetup::UISDeveloperSettings_InputSetup()
{
	CategoryName = TEXT("Plugins");
}

void UISDeveloperSettings_InputSetup::PostInitProperties() // after Config is loaded
{
	Super::PostInitProperties();

	// Use the game project's configged Input Actions (GameProjectInputActions)
	InputActions = GameProjectInputActions;

	if (RuntimeInputActions.Num() > 0)
	{
		// Someone tried to add to this in our constructor or another unexpeced place
		check(0);
	}
}


const UInputAction* UISDeveloperSettings_InputSetup::GetInputAction(const FGameplayTag& InTag) const
{
	if (const TSoftObjectPtr<const UInputAction>* InputActionFound = InputActions.Find(InTag))
	{
		return InputActionFound->LoadSynchronous();
	}

	return nullptr;
}

void UISDeveloperSettings_InputSetup::AddRuntimeInputAction(const FGameplayTag& InGameplayTag, const TSoftObjectPtr<const UInputAction> InInputAction)
{
	if (GameProjectInputActions.Contains(InGameplayTag))
	{
		UE_LOG(LogISDeveloperSettings, Error, TEXT("%s() Tried adding an Input Action with same tag as game project's. Culprit tag: [%s]"), *(InGameplayTag.ToString()));
		check(0);
		return;
	}

	// Manually remove existing Input Action so that the removed delegate gets broadcasted
	if (RuntimeInputActions.Contains(InGameplayTag))
	{
		RemoveRuntimeInputAction(InGameplayTag);
	}

	// Add the Input Action
	const TPair<FGameplayTag, TSoftObjectPtr<const UInputAction>> TagInputActionPair = TPair<FGameplayTag, TSoftObjectPtr<const UInputAction>>(InGameplayTag, InInputAction);

	RuntimeInputActions.Add(TagInputActionPair);
	InputActions.Add(TagInputActionPair);

	OnRuntimeInputActionAdded.Broadcast(TagInputActionPair);
}
void UISDeveloperSettings_InputSetup::RemoveRuntimeInputAction(const FGameplayTag& InGameplayTag)
{
	if (GameProjectInputActions.Contains(InGameplayTag))
	{
		UE_LOG(LogISDeveloperSettings, Error, TEXT("%s() Tried removing a game project's Input Action. Culprit tag: [%s]"), *(InGameplayTag.ToString()));
		check(0);
		return;
	}

	TSoftObjectPtr<const UInputAction>* FoundInputAction = RuntimeInputActions.Find(InGameplayTag);
	if (FoundInputAction)
	{
		RuntimeInputActions.Remove(InGameplayTag);
		InputActions.Remove(InGameplayTag);

		OnRuntimeInputActionRemoved.Broadcast(TPair<FGameplayTag, TSoftObjectPtr<const UInputAction>>(InGameplayTag, FoundInputAction->LoadSynchronous()));
	}
}
