// Fill out your copyright notice in the Description page of Project Settings.

#include "Subsystems/ISEngineSubsystem_ObjectReferenceLibrary.h"

#include "InputAction.h"
#if WITH_EDITOR
#include "ISettingsModule.h"
#endif // #if WITH_EDITOR
#include "Subsystems/ISPrimaryDataAsset_PluginObjectReferenceCollection.h"
#include "Engine/AssetManager.h"
#include "GCUtils_Plugin.h"
#include "Interfaces/IPluginManager.h"
#include "GCUtils_AssetStreaming.h"
#include "GCUtils_AssetStreaming.inl"
#include "GCUtils_Set.h"
#include "GCUtils_Log.h"
#include "GCUtils_String.h"

UISEngineSubsystem_ObjectReferenceLibrary::UISEngineSubsystem_ObjectReferenceLibrary()
{
}

void UISEngineSubsystem_ObjectReferenceLibrary::PostInitProperties()
{
    Super::PostInitProperties();

    ensure(PluginObjectReferenceCollections.Num() <= 0);
}

void UISEngineSubsystem_ObjectReferenceLibrary::Initialize(FSubsystemCollectionBase& inCollection)
{
    Super::Initialize(inCollection);

    UAssetManager::CallOrRegister_OnAssetManagerCreated(
        FSimpleMulticastDelegate::FDelegate::CreateUObject(this, &ThisClass::OnAssetManagerCreated));

#if WITH_EDITOR
    ISettingsModule& settingsModule = FModuleManager::GetModuleChecked<ISettingsModule>(TEXT("Settings"));
    settingsModule.RegisterSettings(
        FName(TEXT("Project")),
        FName(TEXT("Plugins")),
        FName(TEXT("InputSetup")),
        FText::FromString(TEXT("Input Setup")),
        FText::FromString(TEXT("Object Reference Library")),
        this
        );
#endif // #if WITH_EDITOR
}

void UISEngineSubsystem_ObjectReferenceLibrary::Deinitialize()
{
#if WITH_EDITOR
    ISettingsModule& settingsModule = FModuleManager::GetModuleChecked<ISettingsModule>(TEXT("Settings"));
    settingsModule.UnregisterSettings(
        FName(TEXT("Project")),
        FName(TEXT("Plugins")),
        FName(TEXT("InputSetup"))
        );
#endif // #if WITH_EDITOR

    Super::Deinitialize();
}

UISEngineSubsystem_ObjectReferenceLibrary& UISEngineSubsystem_ObjectReferenceLibrary::GetChecked(const UEngine& inEngine)
{
    ThisClass* subsystem = inEngine.GetEngineSubsystem<ThisClass>();
    check(subsystem);
    return *subsystem;
}

FSoftObjectPath UISEngineSubsystem_ObjectReferenceLibrary::GetPluginObjectReferenceCollectionAssetPath(
    const TSharedRef<const IPlugin>& inPlugin)
{
    FStringBuilderBase&& assetName = WriteToString<64>(TEXT("PORC_"), inPlugin->GetName());
    FStringBuilderBase&& packageName = WriteToString<256>(inPlugin->GetMountedAssetPath(), TEXT("Input"), TEXT('/'), assetName);

    return FSoftObjectPath(
        FTopLevelAssetPath(
            FName(MoveTemp(packageName)),
            FName(MoveTemp(assetName))
            )
        );
}

const UInputAction* UISEngineSubsystem_ObjectReferenceLibrary::GetInputAction(const FGameplayTag& inTag) const
{
    const TObjectPtr<const UInputAction>* foundInputAction = InputActionReferences.Find(inTag);
    if (!foundInputAction)
    {
        return nullptr;
    }

    check(*foundInputAction);
    return *foundInputAction;
}

bool UISEngineSubsystem_ObjectReferenceLibrary::TryRegisterInputAction(const FGameplayTag& inTag, const UInputAction* inAsset)
{
    GC_LOG_STR_UOBJECT(
        this,
        LogISObjectReferenceLibrarySubsystem,
        Verbose,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Trying to register new input action by pointer.")
            TEXT(" ")
            TEXT("Gameplay tag: '") << inTag.GetTagName() << TEXT("'.")
            << TEXT(" ")
            TEXT("Input action: '") << GCUtils::String::GetUObjectPathNameSafe(inAsset) << TEXT("'.")
        );

    if (!inAsset)
    {
        return false;
    }

    return TryRegisterInputAction(inTag, *inAsset);
}

bool UISEngineSubsystem_ObjectReferenceLibrary::TryRegisterInputAction(const FGameplayTag& inTag, const UInputAction& inAsset)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UISEngineSubsystem_ObjectReferenceLibrary::RegisterInputAction);

    GC_LOG_STR_UOBJECT(
        this,
        LogISObjectReferenceLibrarySubsystem,
        Verbose,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Trying to register new input action.")
            TEXT(" ")
            TEXT("Gameplay tag: '") << inTag.GetTagName() << TEXT("'.")
            << TEXT(" ")
            TEXT("Input action: '") << GCUtils::String::GetUObjectPathName(inAsset) << TEXT("'.")
        );

    if (const UInputAction* foundInputAction = GetInputAction(inTag))
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogISObjectReferenceLibrarySubsystem,
            Error,
            GCUtils::Materialize(TStringBuilder<512>())
                << TEXT("Caller attempting to add input action with a tag already registered. The existing input action for that tag will be overwritten.")
                TEXT(" ")
                TEXT("Gameplay tag: '") << inTag.GetTagName() << TEXT("'.")
                << TEXT(" ")
                TEXT("Existing input action: '") << GCUtils::String::GetUObjectPathName(*foundInputAction) << TEXT("'.")
                << TEXT(" ")
                TEXT("New input action: '") << GCUtils::String::GetUObjectPathName(inAsset) << TEXT("'.")
            );
        ensure(false);
        return false;
    }

    ensure(InputActionReferences.Contains(inTag) == false);

    GC_LOG_STR_UOBJECT(
        this,
        LogISObjectReferenceLibrarySubsystem,
        Log,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Registering new input action.")
            TEXT(" ")
            TEXT("Gameplay tag: '") << inTag.GetTagName() << TEXT("'.")
            << TEXT(" ")
            TEXT("Input action: '") << GCUtils::String::GetUObjectPathName(inAsset) << TEXT("'.")
        );

    InputActionReferences.Emplace(inTag, &inAsset);
    OnInputActionRegisteredDelegate.Broadcast(inTag, inAsset);
    return true;
}

const UInputAction* UISEngineSubsystem_ObjectReferenceLibrary::UnregisterInputActionByTag(const FGameplayTag& inTag)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UISEngineSubsystem_ObjectReferenceLibrary::UnregisterInputActionByTag);

    GC_LOG_STR_UOBJECT(
        this,
        LogISObjectReferenceLibrarySubsystem,
        Verbose,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Trying to remove input action by tag.")
            TEXT(" ")
            TEXT("Gameplay tag: '") << inTag.GetTagName() << TEXT("'.")
        );

    const TObjectPtr<const UInputAction>* foundInputAction = InputActionReferences.Find(inTag);
    if (!foundInputAction)
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogISObjectReferenceLibrarySubsystem,
            Verbose,
            GCUtils::Materialize(TStringBuilder<512>())
                << TEXT("No input action found for tag: '") << inTag.GetTagName() << TEXT("'.")
            );
        return nullptr;
    }

    check(*foundInputAction);
    const UInputAction& inputAction = **foundInputAction;

    GC_LOG_STR_UOBJECT(
        this,
        LogISObjectReferenceLibrarySubsystem,
        Log,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Unregistering input action.")
            TEXT(" ")
            TEXT("Gameplay tag: '") << inTag.GetTagName() << TEXT("'.")
            << TEXT(" ")
            TEXT("Input action: '") << GCUtils::String::GetUObjectPathName(inputAction) << TEXT("'.")
        );

    const int32 numRemoved = InputActionReferences.Remove(inTag);
    ensure(numRemoved == 1);

    OnInputActionUnregisteredDelegate.Broadcast(inTag, inputAction);

    return &inputAction;
}

bool UISEngineSubsystem_ObjectReferenceLibrary::TryRegisterObjectReferenceCollection(
    const UISPrimaryDataAsset_PluginObjectReferenceCollection& inAsset)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UISEngineSubsystem_ObjectReferenceLibrary::TryRegisterObjectReferenceCollection);

    GC_LOG_STR_UOBJECT(
        this,
        LogISObjectReferenceLibrarySubsystem,
        Log,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Trying to register object reference collection '") << GCUtils::String::GetUObjectPathName(inAsset) << TEXT("'.")
        );

    if (PluginObjectReferenceCollections.Contains(&inAsset))
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogISObjectReferenceLibrarySubsystem,
            Error,
            GCUtils::Materialize(TStringBuilder<512>())
                << TEXT("Object reference collection '") << GCUtils::String::GetUObjectPathName(inAsset) << TEXT("' has already been registered.")
            );
        ensure(false);
        return false;
    }

    // Make sure the data asset doesn't contain any already-added input action references.
    for (const TPair<FGameplayTag, TObjectPtr<UInputAction>>& tagToInputActionPair : inAsset.PluginInputActionReferences)
    {
        const UInputAction* foundInputAction = GetInputAction(tagToInputActionPair.Key);
        if (foundInputAction)
        {
            GC_LOG_STR_UOBJECT(
                this,
                LogISObjectReferenceLibrarySubsystem,
                Error,
                GCUtils::Materialize(TStringBuilder<512>())
                    << TEXT("Caller tried adding an object reference collection which contains a gameplay tag that's already used with a registered input action.")
                    TEXT(" ")
                    TEXT("Object reference collection: '") << GCUtils::String::GetUObjectPathName(inAsset) << TEXT("'.")
                    TEXT(" ")
                    TEXT("Culprit tag: '") << tagToInputActionPair.Key.GetTagName() << TEXT("'.")
                    TEXT(" ")
                    TEXT("Existing input action: '") << GCUtils::String::GetUObjectPathName(*foundInputAction) << TEXT("'.")
                    TEXT(" ")
                    TEXT("Collection's input action: '") << GCUtils::String::GetUObjectPathNameSafe(tagToInputActionPair.Value) << TEXT("'.")
                );
            ensure(false);
            return false;
        }
    }

    // Add the data asset and all of its object references.
    PluginObjectReferenceCollections.Emplace(&inAsset);

    for (const TPair<FGameplayTag, TObjectPtr<UInputAction>>& tagToInputActionPair: inAsset.PluginInputActionReferences)
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogISObjectReferenceLibrarySubsystem,
            Verbose,
            GCUtils::Materialize(TStringBuilder<512>())
                << TEXT("Registering new input action '") << GCUtils::String::GetUObjectPathNameSafe(tagToInputActionPair.Value) << TEXT("' by object reference collection '") << GCUtils::String::GetUObjectPathName(inAsset) << TEXT("'.")
                TEXT(" ")
                TEXT("Input action tag: '") << tagToInputActionPair.Key.GetTagName() << TEXT("'.")
            );

        const bool didRegister = TryRegisterInputAction(tagToInputActionPair.Key, tagToInputActionPair.Value);
        ensure(didRegister);
    }

    return true;
}

bool UISEngineSubsystem_ObjectReferenceLibrary::TryUnregisterObjectReferenceCollection(
    const UISPrimaryDataAsset_PluginObjectReferenceCollection& inAsset)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UISEngineSubsystem_ObjectReferenceLibrary::TryUnregisterObjectReferenceCollection);

    // Remove the data asset and all of its registered input actions.

    GC_LOG_STR_UOBJECT(
        this,
        LogISObjectReferenceLibrarySubsystem,
        Log,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Trying to unregister object reference collection '") << GCUtils::String::GetUObjectPathName(inAsset) << TEXT("'.")
        );

    const int32 numRemoved = PluginObjectReferenceCollections.Remove(&inAsset);
    if (numRemoved <= 0)
    {
        // Nothing to unregister.
        return false;
    }

    ensure(numRemoved == 1);

    for (const TPair<FGameplayTag, TObjectPtr<UInputAction>>& tagToInputActionPair : inAsset.PluginInputActionReferences)
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogISObjectReferenceLibrarySubsystem,
            Verbose,
            GCUtils::Materialize(TStringBuilder<512>())
                << TEXT("Unregistering input action '") << GCUtils::String::GetUObjectPathNameSafe(tagToInputActionPair.Value) << TEXT("' by object reference collection '") << GCUtils::String::GetUObjectPathName(inAsset) << TEXT("'.")
                TEXT(" ")
                TEXT("Input action tag: '") << tagToInputActionPair.Key.GetTagName() << TEXT("'.")
            );

        const bool didUnregister = UnregisterInputActionByTag(tagToInputActionPair.Key) != nullptr;
        ensure(didUnregister);
    }

    return true;
}

void UISEngineSubsystem_ObjectReferenceLibrary::OnAssetManagerCreated()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UISEngineSubsystem_ObjectReferenceLibrary::OnAssetManagerCreated);

    // Load the game project's configged references and add them.
    RegisterGameProjectInputActions(UAssetManager::Get());

    GCUtils::Plugin::UseContentFromDependentPlugins(
        TEXT(UE_PLUGIN_NAME),
        GCUtils::Plugin::FPluginRefNativeDelegate::CreateUObject(this, &ThisClass::OnPluginAddContent),
        GCUtils::Plugin::FPluginRefNativeDelegate::CreateUObject(this, &ThisClass::OnPluginRemoveContent)
        );
}

void UISEngineSubsystem_ObjectReferenceLibrary::RegisterGameProjectInputActions(UAssetManager& inAssetManager)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UISEngineSubsystem_ObjectReferenceLibrary::RegisterGameProjectInputActions);

    // TODO @techdebt Christian: Make single asset load request instead of many individual ones.
    for (const TPair<FGameplayTag, TSoftObjectPtr<const UInputAction>>& tagToInputActionPair : GameProjectInputActionReferences)
    {
        // No need to have the streamable handle hold our loaded assets in memory as we will already
        // store strong references to them ourselves.
        constexpr bool shouldManageActiveHandle = false;

        TSharedRef<FStreamableHandle> streamableHandle =
            GCUtils::AssetStreaming::LoadSyncChecked<shouldManageActiveHandle>(
                inAssetManager.GetStreamableManager(),
                FSoftObjectPath(tagToInputActionPair.Value.ToSoftObjectPath())
                );

        constexpr bool shouldReportErrors = true;
        const UInputAction* loadedInputAction =
            GCUtils::AssetStreaming::GetLoadedAsset<shouldReportErrors, UInputAction>(MoveTemp(streamableHandle));

        TryRegisterInputAction(tagToInputActionPair.Key, loadedInputAction);
    }
}

void UISEngineSubsystem_ObjectReferenceLibrary::OnPluginAddContent(TSharedRef<IPlugin>&& inPlugin)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UISEngineSubsystem_ObjectReferenceLibrary::OnPluginAddContent);

    GC_LOG_STR_UOBJECT(
        this,
        LogISObjectReferenceLibrarySubsystem,
        Log,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Plugin '") << inPlugin->GetName() << TEXT("' content added. Loading and registering object reference collection, if any.")
        );

    // No need to have the streamable handle hold our loaded assets in memory as we will already store strong
    // references to them ourselves.
    constexpr bool shouldManageActiveHandle = false;

    TSharedRef<FStreamableHandle> streamableHandle =
        GCUtils::AssetStreaming::LoadSyncChecked<shouldManageActiveHandle>(
            UAssetManager::Get().GetStreamableManager(),
            GetPluginObjectReferenceCollectionAssetPath(inPlugin)
            );

    constexpr bool shouldReportErrors = false;
    const UISPrimaryDataAsset_PluginObjectReferenceCollection* loadedPluginObjectReferenceCollection =
        GCUtils::AssetStreaming::GetLoadedAsset<shouldReportErrors, UISPrimaryDataAsset_PluginObjectReferenceCollection>(MoveTemp(streamableHandle));

    if (!loadedPluginObjectReferenceCollection)
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogISObjectReferenceLibrarySubsystem,
            Verbose,
            GCUtils::Materialize(TStringBuilder<512>())
                << TEXT("No object reference collection asset found for plugin '") << inPlugin->GetName() << TEXT("'.")
                TEXT(" ")
                TEXT("Asset path searched: '") << streamableHandle->GetDebugName() << TEXT("'.");
            );
        return;
    }

    TryRegisterObjectReferenceCollection(*loadedPluginObjectReferenceCollection);
}

void UISEngineSubsystem_ObjectReferenceLibrary::OnPluginRemoveContent(TSharedRef<IPlugin>&& inPlugin)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UISEngineSubsystem_ObjectReferenceLibrary::OnPluginRemoveContent);

    GC_LOG_STR_UOBJECT(
        this,
        LogISObjectReferenceLibrarySubsystem,
        Log,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Plugin '") << inPlugin->GetName() << TEXT("' content removed. Unregistering object reference collection, if any.")
        );

    FSoftObjectPath pluginObjectReferenceCollectionAssetPath = GetPluginObjectReferenceCollectionAssetPath(inPlugin);

    const TObjectPtr<const UISPrimaryDataAsset_PluginObjectReferenceCollection>* foundPluginObjectReferenceCollection =
        GCUtils::Set::FindByPredicate<TObjectPtr<const UISPrimaryDataAsset_PluginObjectReferenceCollection>>(
            PluginObjectReferenceCollections,
            [&pluginObjectReferenceCollectionAssetPath](const TObjectPtr<const UISPrimaryDataAsset_PluginObjectReferenceCollection>& collection)
            {
                return FSoftObjectPath(collection) == pluginObjectReferenceCollectionAssetPath;
            }
            );

    if (!foundPluginObjectReferenceCollection)
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogISObjectReferenceLibrarySubsystem,
            Verbose,
            GCUtils::Materialize(TStringBuilder<512>())
                << TEXT("No registered object reference collection found for plugin '") << inPlugin->GetName() << TEXT("'.")
            );
        return;
    }

    check(*foundPluginObjectReferenceCollection);
    TryUnregisterObjectReferenceCollection(**foundPluginObjectReferenceCollection);
}
