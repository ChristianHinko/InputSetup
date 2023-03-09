// Fill out your copyright notice in the Description page of Project Settings.


#include "ISEngineSubsystem_InputActions.h"

#include "InputAction.h"
#include "Engine/AssetManager.h"
#include "ISPrimaryDataAsset_PluginInputActions.h"
#if WITH_EDITOR
#include "ISettingsModule.h"
#endif // WITH_EDITOR
#include "BlueprintFunctionLibraries/GCBlueprintFunctionLibrary_ContentTools.h"
#include "Interfaces/IPluginManager.h"



UISEngineSubsystem_InputActions::UISEngineSubsystem_InputActions()
{

}


void UISEngineSubsystem_InputActions::PostInitProperties() // after Config is loaded
{
	Super::PostInitProperties();

	// Load the game project's configged Input Actions and add them to the InputActions
	for (const TPair<FGameplayTag, TSoftObjectPtr<const UInputAction>>& TagInputActionPair : GameProjectInputActions)
	{
		const UInputAction* InputAction = TagInputActionPair.Value.LoadSynchronous();
		InputActions.Add(TPair<FGameplayTag, TWeakObjectPtr<const UInputAction>>(TagInputActionPair.Key, InputAction));
	}

	if (PluginInputActions.Num() > 0)
	{
		// Someone tried to add to this in our constructor or another unexpeced place
		check(0);
	}
}

const UInputAction* UISEngineSubsystem_InputActions::GetInputAction(const FGameplayTag& InTag) const
{
	if (const TWeakObjectPtr<const UInputAction>* FoundInputAction = InputActions.Find(InTag))
	{
		return FoundInputAction->Get();
	}

	return nullptr;
}

void UISEngineSubsystem_InputActions::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

#if WITH_EDITOR
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>(TEXT("Settings"));
	SettingsModule->RegisterSettings(
		FName(TEXT("Project")),
		FName(TEXT("Plugins")),
		FName(TEXT("InputSetup")),
		FText::FromString(TEXT("Input Setup")),
		FText::FromString(TEXT("Input Actions")),
		this
	);
#endif // WITH_EDITOR

	UAssetManager::CallOrRegister_OnAssetManagerCreated(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAssetManagerCreated));
}
void UISEngineSubsystem_InputActions::Deinitialize()
{
	Super::Deinitialize();

#if WITH_EDITOR
	ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>(TEXT("Settings"));
	SettingsModule->UnregisterSettings(
		FName(TEXT("Project")),
		FName(TEXT("Plugins")),
		FName(TEXT("InputSetup"))
	);
#endif // WITH_EDITOR
}

void UISEngineSubsystem_InputActions::OnAssetManagerCreated()
{
	UGCBlueprintFunctionLibrary_ContentTools::UseContentFromDependentPlugins(UE_PLUGIN_NAME,
		TDelegate<void(const IPlugin&)>::CreateUObject(this, &ThisClass::OnPluginAddContent),
		TDelegate<void(const IPlugin&)>::CreateUObject(this, &ThisClass::OnPluginRemoveContent)
	);
}

FSoftObjectPath UISEngineSubsystem_InputActions::GetPluginInputActionsAssetPath(const IPlugin& InPlugin)
{
	return FSoftObjectPath(InPlugin.GetMountedAssetPath() / TEXT("Input") / TEXT("PDA_InputActions.PDA_InputActions"));
}

void UISEngineSubsystem_InputActions::OnPluginAddContent(const IPlugin& InPlugin)
{
	const FSoftObjectPath& PluginInputActionsAssetPath = GetPluginInputActionsAssetPath(InPlugin);
	const UObject* LoadedAsset = UAssetManager::GetIfValid()->GetStreamableManager().LoadSynchronous(PluginInputActionsAssetPath);
	const UISPrimaryDataAsset_PluginInputActions* PluginInputActionsAsset = Cast<UISPrimaryDataAsset_PluginInputActions>(LoadedAsset);

	if (!IsValid(PluginInputActionsAsset))
	{
		return;
	}

	// Make sure we don't already have this data asset
	if (PluginInputActions.Contains(PluginInputActionsAsset))
	{
		UE_LOG(LogISInputActionsSubsystem, Error, TEXT("%s() Input Actions data asset has already been added. Aborting. Culprit data asset [%s]"), ANSI_TO_TCHAR(__FUNCTION__), *(PluginInputActionsAsset->GetFName().ToString()));
		check(0);
		return;
	}

	// Make sure the data asset doesn't contain any already-added Input Actions
	for (const TPair<FGameplayTag, TWeakObjectPtr<const UInputAction>>& PluginInputAction : PluginInputActionsAsset->InputActionsToAdd)
	{
		if (InputActions.Contains(PluginInputAction.Key))
		{
			UE_LOG(LogISInputActionsSubsystem, Error, TEXT("%s() Tried adding Input Actions data asset but it contains an Input Action tag that already exists. Aborting. Culprit tag: [%s] from data asset [%s]"), ANSI_TO_TCHAR(__FUNCTION__), *(PluginInputAction.Key.ToString()), *(PluginInputActionsAsset->GetFName().ToString()));
			check(0);
			return;
		}
	}
	// Add the data asset and all of its Input Actions
	UE_LOG(LogISInputActionsSubsystem, Log, TEXT("%s() Adding plugin Input Actions data asset [%s]."), ANSI_TO_TCHAR(__FUNCTION__), *(PluginInputActionsAsset->GetFName().ToString()));

	PluginInputActions.Add(PluginInputActionsAsset);
	for (const TPair<FGameplayTag, TWeakObjectPtr<const UInputAction>>& PluginInputAction : PluginInputActionsAsset->InputActionsToAdd)
	{
		InputActions.Add(PluginInputAction);

		OnPluginInputActionAdded.Broadcast(PluginInputAction);
		UE_LOG(LogISInputActionsSubsystem, Log, TEXT("%s() New plugin Input Action [%s] added by data asset [%s]."), ANSI_TO_TCHAR(__FUNCTION__), *(PluginInputAction.Key.ToString()), *(PluginInputActionsAsset->GetFName().ToString()));
	}
}
void UISEngineSubsystem_InputActions::OnPluginRemoveContent(const IPlugin& InPlugin)
{
	const FSoftObjectPath& PluginInputActionsAssetPath = GetPluginInputActionsAssetPath(InPlugin);
	const UObject* LoadedAsset = UAssetManager::GetIfValid()->GetStreamableManager().LoadSynchronous(PluginInputActionsAssetPath);
	const UISPrimaryDataAsset_PluginInputActions* PluginInputActionsAsset = Cast<UISPrimaryDataAsset_PluginInputActions>(LoadedAsset);

	if (!IsValid(PluginInputActionsAsset))
	{
		return;
	}

	// Make sure we have this data asset
	if (PluginInputActions.Contains(PluginInputActionsAsset) == false)
	{
		UE_LOG(LogISInputActionsSubsystem, Error, TEXT("%s() Input Actions data asset [%s] does not exist in the set."), ANSI_TO_TCHAR(__FUNCTION__), *(PluginInputActionsAsset->GetFName().ToString()));
		check(0);
		return;
	}
	// Remove the data asset and all of its Input Actions
	UE_LOG(LogISInputActionsSubsystem, Log, TEXT("%s() Removing Input Actions data asset [%s] at plugin."), ANSI_TO_TCHAR(__FUNCTION__), *(PluginInputActionsAsset->GetFName().ToString()));

	PluginInputActions.Remove(PluginInputActionsAsset);
	for (const TPair<FGameplayTag, TWeakObjectPtr<const UInputAction>>& PluginInputAction : PluginInputActionsAsset->InputActionsToAdd)
	{
		InputActions.Remove(PluginInputAction.Key);

		OnPluginInputActionRemoved.Broadcast(PluginInputAction);
		UE_LOG(LogISInputActionsSubsystem, Log, TEXT("%s() Plugin Input Action [%s] removed at by data asset [%s]."), ANSI_TO_TCHAR(__FUNCTION__), *(PluginInputAction.Key.ToString()), *(PluginInputActionsAsset->GetFName().ToString()));
	}
}
