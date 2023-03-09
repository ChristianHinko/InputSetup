// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Subsystems/EngineSubsystem.h"

#include "ISEngineSubsystem_InputActions.generated.h"


struct FGameplayTag;
class UInputAction;
class UISPrimaryDataAsset_PluginInputActions;
class IPlugin;



DECLARE_MULTICAST_DELEGATE_OneParam(FISInputActionSetupDelegate, const TPair<PREPROCESSOR_COMMA_SEPARATED(FGameplayTag, TWeakObjectPtr<const UInputAction>)>&);


/**
 * Subsystem holding all Input Actions for your game (any plugin may contribute to the Input Actions via UISPrimaryDataAsset_PluginInputActions).
 * 
 * Adding Input Action(s) from your game: Configure InputSetup in project settings
 * Adding Input Action(s) from a plugin:  UISPrimaryDataAsset_PluginInputActions
 */
UCLASS(config=InputActions)
class INPUTSETUP_API UISEngineSubsystem_InputActions : public UEngineSubsystem
{
	GENERATED_BODY()

private:
	/** Combined map of both GameProjectInputActions and PluginInputActions */
	UPROPERTY(Transient)
		TMap<FGameplayTag, TWeakObjectPtr<const UInputAction>> InputActions;

	/** Game project Input Actions added via project settings */
	UPROPERTY(EditDefaultsOnly, Config)
		TMap<FGameplayTag, TSoftObjectPtr<const UInputAction>> GameProjectInputActions;
	/** To allow plugins to contribute to the InputActions via "Input/PDA_InputActions.uasset */
	UPROPERTY(VisibleDefaultsOnly, DisplayName = "Plugin Input Actions (readonly)")
		TSet<TObjectPtr<const UISPrimaryDataAsset_PluginInputActions>> PluginInputActions;

public:
	UISEngineSubsystem_InputActions();

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;


	/** Get all Input Actions. These are paired with Gameplay Tags which identify them. */
	const TMap<FGameplayTag, TWeakObjectPtr<const UInputAction>>& GetInputActions() const { return InputActions; }

	/** Get an Input Action using its Gameplay Tag. */
	UFUNCTION(BlueprintPure, Category = "InputActions")
		const UInputAction* GetInputAction(const FGameplayTag& InTag) const;


	mutable FISInputActionSetupDelegate OnPluginInputActionAdded;
	mutable FISInputActionSetupDelegate OnPluginInputActionRemoved;

	static FSoftObjectPath GetPluginInputActionsAssetPath(const IPlugin& InPlugin);

protected:
	virtual void PostInitProperties() override; // after Config is loaded

	void OnAssetManagerCreated();

	void OnPluginAddContent(const IPlugin& InPlugin);
	void OnPluginRemoveContent(const IPlugin& InPlugin);
};
