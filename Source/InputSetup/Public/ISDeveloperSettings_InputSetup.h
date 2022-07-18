// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DeveloperSettings.h"

#include "ISDeveloperSettings_InputSetup.generated.h"


struct FGameplayTag;
class UInputAction;



DECLARE_MULTICAST_DELEGATE_OneParam(FISInputActionSetupDelegate, const TPair<PREPROCESSOR_COMMA_SEPARATED(FGameplayTag, TSoftObjectPtr<const UInputAction>>&));


/**
 * This CDO is the centralize place to access all Input Actions (accessable in game and plugins).
 */
UCLASS(config=InputSetupDeveloperSettings, meta=(DisplayName="Input Setup"))
class INPUTSETUP_API UISDeveloperSettings_InputSetup : public UDeveloperSettings
{
	GENERATED_BODY()

private:
	/** Combined map of both GameProjectInputActions and RuntimeInputActions */
	TMap<FGameplayTag, TSoftObjectPtr<const UInputAction>> InputActions;

	/**
	 * For the game project to add Input Actions via Config (through the project settings).
	 */
	UPROPERTY(EditDefaultsOnly, Config)
		TMap<FGameplayTag, TSoftObjectPtr<const UInputAction>> GameProjectInputActions;
	/**
	 * For code to add Input Actions during runtime.
	 * This allows non-gameproject modules to contribute to the InputActions.
	 */
	UPROPERTY(VisibleDefaultsOnly)
		TMap<FGameplayTag, TSoftObjectPtr<const UInputAction>> RuntimeInputActions;

public:
	UISDeveloperSettings_InputSetup();


	/** Get all Input Actions. These are paired with Gameplay Tags which identify them. */
	const TMap<FGameplayTag, TSoftObjectPtr<const UInputAction>>& GetInputActions() const { return InputActions; }

	/** Use an Input Action tag to get an Input Action. */
	const UInputAction* GetInputAction(const FGameplayTag& InTag) const;

	/** A common callsite: IModuleInterface::StartupModule() */
	void AddRuntimeInputAction(const FGameplayTag& InGameplayTag, const TSoftObjectPtr<const UInputAction> InInputAction);
	/** A common callsite: IModuleInterface::ShutdownModule() */
	void RemoveRuntimeInputAction(const FGameplayTag& InGameplayTag);

	/** Broadcasted when Input Actions, not known by compile time, are added */
	mutable FISInputActionSetupDelegate OnRuntimeInputActionAdded;
	/** Broadcasted when Input Actions, not known by compile time, are removed */
	mutable FISInputActionSetupDelegate OnRuntimeInputActionRemoved;

protected:
	virtual void PostInitProperties() override; // after Config is loaded
};
