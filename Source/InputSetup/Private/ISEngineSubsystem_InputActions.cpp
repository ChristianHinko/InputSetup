// Fill out your copyright notice in the Description page of Project Settings.


#include "ISEngineSubsystem_InputActions.h"

#include "InputAction.h"
#include "Interfaces/IPluginManager.h"
#include "PluginDescriptor.h"
#include "Engine/AssetManager.h"
#include "ISPrimaryDataAsset_PluginInputActions.h"
#if WITH_EDITOR
#include "ISettingsModule.h"
#endif // WITH_EDITOR



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
	// Check for plugins with PluginInputActions
	for (const TSharedRef<IPlugin>& Plugin : IPluginManager::Get().GetEnabledPluginsWithContent())
	{
		if (FPackageName::MountPointExists(Plugin->GetMountedAssetPath())) // if the plugin's content is mounted
		{
			const UISPrimaryDataAsset_PluginInputActions* PluginPluginInputActions = TryGetPluginInputActionsFromPlugin(Plugin);
			if (IsValid(PluginPluginInputActions))
			{
				AddPluginInputActions(PluginPluginInputActions);
			}
		}
	}

	// Listen for dynamically loaded and unloaded plugins, e.g., Game Features
	FPackageName::OnContentPathMounted().AddWeakLambda(this,
		[this](const FString& InAssetPath, const FString& InContentPath)
		{
			// See if there is a new PluginInputAcions to add
			const TSharedPtr<IPlugin>& Plugin = IPluginManager::Get().FindPluginFromPath(InAssetPath);
			if (Plugin.IsValid())
			{
				AddPluginInputActions(TryGetPluginInputActionsFromPlugin(Plugin.ToSharedRef()));
			}
		}
	);
	FPackageName::OnContentPathDismounted().AddWeakLambda(this,
		[this](const FString& InAssetPath, const FString& InContentPath)
		{
			// See if there is a PluginInputAcions to remove
			const TSharedPtr<IPlugin>& Plugin = IPluginManager::Get().FindPluginFromPath(InAssetPath);
			if (Plugin.IsValid())
			{
				RemovePluginInputActions(TryGetPluginInputActionsFromPlugin(Plugin.ToSharedRef()));
			}
		}
	);
}

const UISPrimaryDataAsset_PluginInputActions* UISEngineSubsystem_InputActions::TryGetPluginInputActionsFromPlugin(const TSharedRef<IPlugin>& InPlugin)
{
	// Load the PluginInputActions data asset from the plugin if they depend on us
	if (InPlugin->GetDescriptor().Plugins.ContainsByPredicate(
			[](const FPluginReferenceDescriptor& InPluginReferenceDescriptor)
			{
				return (InPluginReferenceDescriptor.bEnabled) && (InPluginReferenceDescriptor.Name == TEXT(UE_PLUGIN_NAME));
			})
		)
	{
		const FSoftObjectPath InputActionsDataAssetPath = InPlugin->GetMountedAssetPath() / TEXT("Input") / TEXT("PDA_InputActions.PDA_InputActions");
		const UObject* LoadedObject = UAssetManager::GetIfValid()->GetStreamableManager().LoadSynchronous(InputActionsDataAssetPath);
		const UISPrimaryDataAsset_PluginInputActions* InputActionsDataAsset = Cast<UISPrimaryDataAsset_PluginInputActions>(LoadedObject);
		return InputActionsDataAsset;
	}

	return nullptr;
}

void UISEngineSubsystem_InputActions::AddPluginInputActions(const UISPrimaryDataAsset_PluginInputActions* InPluginInputActions)
{
	if (!IsValid(InPluginInputActions))
	{
		return;
	}

	// Make sure we don't already have this data asset
	if (PluginInputActions.Contains(InPluginInputActions))
	{
		UE_LOG(LogISInputActionsSubsystem, Error, TEXT("%s() Input Actions data asset has already been added. Aborting. Culprit data asset [%s]"), ANSI_TO_TCHAR(__FUNCTION__), *(InPluginInputActions->GetFName().ToString()));
		check(0);
		return;
	}

	// Make sure the data asset doesn't contain any already-added Input Actions
	for (const TPair<FGameplayTag, TWeakObjectPtr<const UInputAction>>& PluginInputAction : InPluginInputActions->InputActionsToAdd)
	{
		if (InputActions.Contains(PluginInputAction.Key))
		{
			UE_LOG(LogISInputActionsSubsystem, Error, TEXT("%s() Tried adding Input Actions data asset but it contains an Input Action tag that already exists. Aborting. Culprit tag: [%s] from data asset [%s]"), ANSI_TO_TCHAR(__FUNCTION__), *(PluginInputAction.Key.ToString()), *(InPluginInputActions->GetFName().ToString()));
			check(0);
			return;
		}
	}
	// Add the data asset and all of its Input Actions
	UE_LOG(LogISInputActionsSubsystem, Log, TEXT("%s() Adding plugin Input Actions data asset [%s]."), ANSI_TO_TCHAR(__FUNCTION__), *(InPluginInputActions->GetFName().ToString()));
	
	PluginInputActions.Add(InPluginInputActions);
	for (const TPair<FGameplayTag, TWeakObjectPtr<const UInputAction>>& PluginInputAction : InPluginInputActions->InputActionsToAdd)
	{
		InputActions.Add(PluginInputAction);

		OnPluginInputActionAdded.Broadcast(PluginInputAction);
		UE_LOG(LogISInputActionsSubsystem, Log, TEXT("%s() New plugin Input Action [%s] added by data asset [%s]."), ANSI_TO_TCHAR(__FUNCTION__), *(PluginInputAction.Key.ToString()), *(InPluginInputActions->GetFName().ToString()));
	}
}
void UISEngineSubsystem_InputActions::RemovePluginInputActions(const UISPrimaryDataAsset_PluginInputActions* InPluginInputActions)
{
	if (!IsValid(InPluginInputActions))
	{
		return;
	}

	// Make sure we have this data asset
	if (PluginInputActions.Contains(InPluginInputActions) == false)
	{
		UE_LOG(LogISInputActionsSubsystem, Error, TEXT("%s() Input Actions data asset [%s] does not exist in the set."), ANSI_TO_TCHAR(__FUNCTION__), *(InPluginInputActions->GetFName().ToString()));
		check(0);
		return;
	}
	// Remove the data asset and all of its Input Actions
	UE_LOG(LogISInputActionsSubsystem, Log, TEXT("%s() Removing Input Actions data asset [%s] at plugin."), ANSI_TO_TCHAR(__FUNCTION__), *(InPluginInputActions->GetFName().ToString()));

	PluginInputActions.Remove(InPluginInputActions);
	for (const TPair<FGameplayTag, TWeakObjectPtr<const UInputAction>>& PluginInputAction : InPluginInputActions->InputActionsToAdd)
	{
		InputActions.Remove(PluginInputAction.Key);

		OnPluginInputActionRemoved.Broadcast(PluginInputAction);
		UE_LOG(LogISInputActionsSubsystem, Log, TEXT("%s() Plugin Input Action [%s] removed at by data asset [%s]."), ANSI_TO_TCHAR(__FUNCTION__), *(PluginInputAction.Key.ToString()), *(InPluginInputActions->GetFName().ToString()));
	}
}
