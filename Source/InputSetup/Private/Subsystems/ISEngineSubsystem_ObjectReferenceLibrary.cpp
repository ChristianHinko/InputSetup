// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/ISEngineSubsystem_ObjectReferenceLibrary.h"

#include "InputAction.h"
#if WITH_EDITOR
#include "ISettingsModule.h"
#endif // WITH_EDITOR
#include "Subsystems/ISPrimaryDataAsset_PluginObjectReferenceCollection.h"
#include "Engine/AssetManager.h"
#include "GCUtils_Plugin.h"
#include "Interfaces/IPluginManager.h"



UISEngineSubsystem_ObjectReferenceLibrary::UISEngineSubsystem_ObjectReferenceLibrary()
{
}


void UISEngineSubsystem_ObjectReferenceLibrary::PostInitProperties()
{
    Super::PostInitProperties();

    // Load the game project's configged references and add them
    for (const TPair<FGameplayTag, TSoftObjectPtr<const UInputAction>>& TagInputActionPair : GameProjectInputActionReferences)
    {
        const UInputAction* InputAction = TagInputActionPair.Value.LoadSynchronous();
        InputActionReferences.Add(TPair<FGameplayTag, TObjectPtr<const UInputAction>>(TagInputActionPair.Key, InputAction));
    }

    check(PluginObjectReferenceCollections.Num() <= 0)
}

void UISEngineSubsystem_ObjectReferenceLibrary::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

#if WITH_EDITOR
    ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>(TEXT("Settings"));
    SettingsModule->RegisterSettings(
            FName(TEXT("Project")),
            FName(TEXT("Plugins")),
            FName(TEXT("InputSetup")),
            FText::FromString(TEXT("Input Setup")),
            FText::FromString(TEXT("Object Reference Library")),
            this
        );
#endif // WITH_EDITOR

    UAssetManager::CallOrRegister_OnAssetManagerCreated(FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAssetManagerCreated));
}
void UISEngineSubsystem_ObjectReferenceLibrary::Deinitialize()
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

const UInputAction* UISEngineSubsystem_ObjectReferenceLibrary::GetInputAction(const FGameplayTag& InPortrayalTag) const
{
    if (const TWeakObjectPtr<const UInputAction>* FoundInputAction = InputActionReferences.Find(InPortrayalTag))
    {
        return FoundInputAction->Get();
    }

    return nullptr;
}

void UISEngineSubsystem_ObjectReferenceLibrary::OnAssetManagerCreated()
{
    GCUtils::Plugin::UseContentFromDependentPlugins(
        TEXT(UE_PLUGIN_NAME),
        GCUtils::Plugin::FPluginRefNativeDelegate::CreateUObject(this, &ThisClass::OnPluginAddContent),
        GCUtils::Plugin::FPluginRefNativeDelegate::CreateUObject(this, &ThisClass::OnPluginRemoveContent)
        );
}

FSoftObjectPath UISEngineSubsystem_ObjectReferenceLibrary::GetPluginObjectReferenceCollectionAssetPath(const TSharedRef<const IPlugin>& InPlugin)
{
    const FString AssetName = (TEXT("PORC_") + InPlugin->GetName());
    return FSoftObjectPath(InPlugin->GetMountedAssetPath() / TEXT("Input") / AssetName + TEXT('.') + AssetName);
}

void UISEngineSubsystem_ObjectReferenceLibrary::OnPluginAddContent(TSharedRef<IPlugin>&& InPlugin)
{
    const FSoftObjectPath& PluginObjectReferenceCollectionAssetPath = GetPluginObjectReferenceCollectionAssetPath(InPlugin);
    const UObject* LoadedAsset = UAssetManager::Get().GetStreamableManager().LoadSynchronous(PluginObjectReferenceCollectionAssetPath);
    const UISPrimaryDataAsset_PluginObjectReferenceCollection* PluginObjectReferenceCollection = Cast<UISPrimaryDataAsset_PluginObjectReferenceCollection>(LoadedAsset);

    if (!IsValid(PluginObjectReferenceCollection))
    {
        return;
    }

    if (PluginObjectReferenceCollections.Contains(PluginObjectReferenceCollection))
    {
        UE_LOG(LogISObjectReferenceLibrarySubsystem, Error, TEXT("%s() PluginObjectReferenceCollection [%s] has already been added."), ANSI_TO_TCHAR(__FUNCTION__), *(PluginObjectReferenceCollection->GetName()));
        check(0);
        return;
    }

    // Make sure the data asset doesn't contain any already-added InputAction references
    for (const TPair<FGameplayTag, TObjectPtr<UInputAction>>& PluginInputActionReference : PluginObjectReferenceCollection->PluginInputActionReferences)
    {
        if (InputActionReferences.Contains(PluginInputActionReference.Key))
        {
            UE_LOG(LogISObjectReferenceLibrarySubsystem, Error, TEXT("%s() Tried adding PluginObjectReferenceCollection but it contains a gameplay that is already being used. Aborting. Culprit tag [%s] from data asset [%s]."), ANSI_TO_TCHAR(__FUNCTION__), *(PluginInputActionReference.Key.ToString()), *(PluginObjectReferenceCollection->GetName()));
            check(0);
            return;
        }
    }

    // Add the data asset and all of its object references
    UE_LOG(LogISObjectReferenceLibrarySubsystem, Log, TEXT("%s() Adding PluginObjectReferenceCollection [%s]."), ANSI_TO_TCHAR(__FUNCTION__), *(PluginObjectReferenceCollection->GetName()));

    PluginObjectReferenceCollections.Add(PluginObjectReferenceCollection);
    for (const TPair<FGameplayTag, TObjectPtr<UInputAction>>& PluginInputActionReference : PluginObjectReferenceCollection->PluginInputActionReferences)
    {
        InputActionReferences.Add(TPair<FGameplayTag, TObjectPtr<const UInputAction>>(PluginInputActionReference.Key, PluginInputActionReference.Value));

        OnInputActionAdded.Broadcast(PluginInputActionReference);

        UE_LOG(LogISObjectReferenceLibrarySubsystem, Log, TEXT("%s() New plugin InputAction reference [%s] added by data asset [%s]."), ANSI_TO_TCHAR(__FUNCTION__), *(PluginInputActionReference.Key.ToString()), *(PluginObjectReferenceCollection->GetName()));
    }
}

void UISEngineSubsystem_ObjectReferenceLibrary::OnPluginRemoveContent(TSharedRef<IPlugin>&& InPlugin)
{
    const FSoftObjectPath& PluginObjectReferenceCollectionAssetPath = GetPluginObjectReferenceCollectionAssetPath(InPlugin);
    const UObject* LoadedAsset = UAssetManager::Get().GetStreamableManager().LoadSynchronous(PluginObjectReferenceCollectionAssetPath);
    const UISPrimaryDataAsset_PluginObjectReferenceCollection* PluginObjectReferenceCollection = Cast<UISPrimaryDataAsset_PluginObjectReferenceCollection>(LoadedAsset);

    if (!IsValid(PluginObjectReferenceCollection))
    {
        return;
    }

    if (PluginObjectReferenceCollections.Contains(PluginObjectReferenceCollection) == false)
    {
        UE_LOG(LogISObjectReferenceLibrarySubsystem, Error, TEXT("%s() PluginObjectReferenceCollection [%s] does not exist in the set. Nothing to remove."), ANSI_TO_TCHAR(__FUNCTION__), *(PluginObjectReferenceCollection->GetName()));
        check(0);
        return;
    }

    // Remove the data asset and all of its InputAction references
    UE_LOG(LogISObjectReferenceLibrarySubsystem, Log, TEXT("%s() Removing PluginObjectReferenceCollection [%s]."), ANSI_TO_TCHAR(__FUNCTION__), *(PluginObjectReferenceCollection->GetName()));

    PluginObjectReferenceCollections.Remove(PluginObjectReferenceCollection);
    for (const TPair<FGameplayTag, TObjectPtr<UInputAction>>& PluginInputActionReference : PluginObjectReferenceCollection->PluginInputActionReferences)
    {
        InputActionReferences.Remove(PluginInputActionReference.Key);

        OnInputActionRemoved.Broadcast(PluginInputActionReference);

        UE_LOG(LogISObjectReferenceLibrarySubsystem, Log, TEXT("%s() Plugin InputAction reference [%s] removed at by data asset [%s]."), ANSI_TO_TCHAR(__FUNCTION__), *(PluginInputActionReference.Key.ToString()), *(PluginObjectReferenceCollection->GetName()));
    }
}
