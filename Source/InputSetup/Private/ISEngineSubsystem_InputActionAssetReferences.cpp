// Fill out your copyright notice in the Description page of Project Settings.

#include "ISEngineSubsystem_InputActionAssetReferences.h"

#include "InputAction.h"
#if WITH_EDITOR
#include "ISettingsModule.h"
#endif // #if WITH_EDITOR
#include "ISPrimaryDataAsset_InputActionAssetReferences.h"
#include "Engine/AssetManager.h"
#include "GCUtils_Plugin.h"
#include "Interfaces/IPluginManager.h"
#include "GCUtils_AssetStreaming.h"
#include "GCUtils_AssetStreaming.inl"
#include "GCUtils_Set.h"
#include "GCUtils_Log.h"
#include "GCUtils_String.h"

DEFINE_LOG_CATEGORY_STATIC(LogISEngineSubsystem_InputActionAssetReferences, Log, All);

UISEngineSubsystem_InputActionAssetReferences::UISEngineSubsystem_InputActionAssetReferences()
{
}

void UISEngineSubsystem_InputActionAssetReferences::PostInitProperties()
{
    Super::PostInitProperties();
}

void UISEngineSubsystem_InputActionAssetReferences::Initialize(FSubsystemCollectionBase& inCollection)
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
        FText::FromString(TEXT("Input Action Asset References")),
        this
        );
#endif // #if WITH_EDITOR
}

void UISEngineSubsystem_InputActionAssetReferences::Deinitialize()
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

UISEngineSubsystem_InputActionAssetReferences& UISEngineSubsystem_InputActionAssetReferences::GetChecked(const UEngine& inEngine)
{
    ThisClass* subsystem = inEngine.GetEngineSubsystem<ThisClass>();
    check(subsystem);
    return *subsystem;
}

FSoftObjectPath UISEngineSubsystem_InputActionAssetReferences::GetAssetReferenceDataAssetPathForPlugin(
    const TSharedRef<const IPlugin>& inPlugin)
{
    FStringBuilderBase&& assetName = WriteToString<64>(TEXT("IAAR_"), inPlugin->GetName());
    FStringBuilderBase&& packageName = WriteToString<256>(inPlugin->GetMountedAssetPath(), TEXT("Input"), TEXT('/'), assetName);

    return FSoftObjectPath(
        FTopLevelAssetPath(
            FName(MoveTemp(packageName)),
            FName(MoveTemp(assetName))
            )
        );
}

const UInputAction* UISEngineSubsystem_InputActionAssetReferences::GetInputAction(const FGameplayTag& inTag) const
{
    const TObjectPtr<const UInputAction>* foundInputAction = ReferencedInputActions.Find(inTag);
    if (!foundInputAction)
    {
        return nullptr;
    }

    check(*foundInputAction);
    return *foundInputAction;
}

bool UISEngineSubsystem_InputActionAssetReferences::TryAddReferencedInputAction(const FGameplayTag& inTag, const UInputAction* inAsset)
{
    GC_LOG_STR_UOBJECT(
        this,
        LogISEngineSubsystem_InputActionAssetReferences,
        Verbose,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Trying to add new referenced asset by pointer.")
            TEXT(" ")
            TEXT("Gameplay tag: '") << inTag.GetTagName() << TEXT("'.")
            << TEXT(" ")
            TEXT("Referenced asset: '") << GCUtils::String::GetUObjectPathNameSafe(inAsset) << TEXT("'.")
        );

    if (!inAsset)
    {
        return false;
    }

    return TryAddReferencedInputAction(inTag, *inAsset);
}

bool UISEngineSubsystem_InputActionAssetReferences::TryAddReferencedInputAction(const FGameplayTag& inTag, const UInputAction& inAsset)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UISEngineSubsystem_InputActionAssetReferences::TryAddReferencedInputAction);

    GC_LOG_STR_UOBJECT(
        this,
        LogISEngineSubsystem_InputActionAssetReferences,
        Verbose,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Trying to add new referenced asset.")
            TEXT(" ")
            TEXT("Gameplay tag: '") << inTag.GetTagName() << TEXT("'.")
            << TEXT(" ")
            TEXT("Referenced asset: '") << GCUtils::String::GetUObjectPathName(inAsset) << TEXT("'.")
        );

    if (const UInputAction* foundInputAction = GetInputAction(inTag))
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogISEngineSubsystem_InputActionAssetReferences,
            Error,
            GCUtils::Materialize(TStringBuilder<512>())
                << TEXT("Caller attempting to add referenced asset with a tag already used. Aborting and returning false.")
                TEXT(" ")
                TEXT("Gameplay tag: '") << inTag.GetTagName() << TEXT("'.")
                << TEXT(" ")
                TEXT("Existing referenced asset: '") << GCUtils::String::GetUObjectPathName(*foundInputAction) << TEXT("'.")
                << TEXT(" ")
                TEXT("Attemped new referenced asset: '") << GCUtils::String::GetUObjectPathName(inAsset) << TEXT("'.")
            );
        ensure(false);
        return false;
    }

    ensure(ReferencedInputActions.Contains(inTag) == false);

    GC_LOG_STR_UOBJECT(
        this,
        LogISEngineSubsystem_InputActionAssetReferences,
        Log,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Adding new referenced asset.")
            TEXT(" ")
            TEXT("Gameplay tag: '") << inTag.GetTagName() << TEXT("'.")
            << TEXT(" ")
            TEXT("Referenced asset: '") << GCUtils::String::GetUObjectPathName(inAsset) << TEXT("'.")
        );

    ReferencedInputActions.Emplace(inTag, &inAsset);
    OnInputActionAddedDelegate.Broadcast(inTag, inAsset);
    return true;
}

const UInputAction* UISEngineSubsystem_InputActionAssetReferences::TryRemoveReferencedInputAction(const FGameplayTag& inTag)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UISEngineSubsystem_InputActionAssetReferences::TryRemoveReferencedInputAction);

    GC_LOG_STR_UOBJECT(
        this,
        LogISEngineSubsystem_InputActionAssetReferences,
        Verbose,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Trying to remove referenced asset by tag.")
            TEXT(" ")
            TEXT("Gameplay tag: '") << inTag.GetTagName() << TEXT("'.")
        );

    const TObjectPtr<const UInputAction>* foundInputAction = ReferencedInputActions.Find(inTag);
    if (!foundInputAction)
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogISEngineSubsystem_InputActionAssetReferences,
            Verbose,
            GCUtils::Materialize(TStringBuilder<512>())
                << TEXT("No referenced asset found for tag: '") << inTag.GetTagName() << TEXT("'.")
            );
        return nullptr;
    }

    check(*foundInputAction);
    const UInputAction& inputAction = **foundInputAction;

    GC_LOG_STR_UOBJECT(
        this,
        LogISEngineSubsystem_InputActionAssetReferences,
        Log,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Removing referenced asset.")
            TEXT(" ")
            TEXT("Gameplay tag: '") << inTag.GetTagName() << TEXT("'.")
            << TEXT(" ")
            TEXT("Referenced asset: '") << GCUtils::String::GetUObjectPathName(inputAction) << TEXT("'.")
        );

    const int32 numRemoved = ReferencedInputActions.Remove(inTag);
    ensure(numRemoved == 1);

    OnInputActionRemovedDelegate.Broadcast(inTag, inputAction);

    return &inputAction;
}

bool UISEngineSubsystem_InputActionAssetReferences::TryAddReferencedAssetsDataAsset(
    const UISPrimaryDataAsset_InputActionAssetReferences& inDataAsset)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UISEngineSubsystem_InputActionAssetReferences::TryAddReferencedAssetsDataAsset);

    GC_LOG_STR_UOBJECT(
        this,
        LogISEngineSubsystem_InputActionAssetReferences,
        Log,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Trying to add asset references data asset'") << GCUtils::String::GetUObjectPathName(inDataAsset) << TEXT("'.")
        );

    if (AssetReferencesDataAssetSet.Contains(&inDataAsset))
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogISEngineSubsystem_InputActionAssetReferences,
            Error,
            GCUtils::Materialize(TStringBuilder<512>())
                << TEXT("Asset references data asset '") << GCUtils::String::GetUObjectPathName(inDataAsset) << TEXT("' has already been added.")
            );
        ensure(false);
        return false;
    }

    // Make sure the data asset doesn't contain any already-added referenced assets.
    for (const TPair<FGameplayTag, TObjectPtr<UInputAction>>& tagToInputActionPair : inDataAsset.InputActionReferences)
    {
        const UInputAction* foundInputAction = GetInputAction(tagToInputActionPair.Key);
        if (foundInputAction)
        {
            GC_LOG_STR_UOBJECT(
                this,
                LogISEngineSubsystem_InputActionAssetReferences,
                Error,
                GCUtils::Materialize(TStringBuilder<512>())
                    << TEXT("Caller tried adding a asset references data asset which contains a gameplay tag that's already used with a referenced asset.")
                    TEXT(" ")
                    TEXT("Asset referennces data asset: '") << GCUtils::String::GetUObjectPathName(inDataAsset) << TEXT("'.")
                    TEXT(" ")
                    TEXT("Culprit tag: '") << tagToInputActionPair.Key.GetTagName() << TEXT("'.")
                    TEXT(" ")
                    TEXT("Existing referenced asset: '") << GCUtils::String::GetUObjectPathName(*foundInputAction) << TEXT("'.")
                    TEXT(" ")
                    TEXT("Data asset's asset reference: '") << GCUtils::String::GetUObjectPathNameSafe(tagToInputActionPair.Value) << TEXT("'.")
                );
            ensure(false);
            return false;
        }
    }

    // Add the data asset and all of its asset references.

    AssetReferencesDataAssetSet.Emplace(&inDataAsset);

    for (const TPair<FGameplayTag, TObjectPtr<UInputAction>>& tagToInputActionPair: inDataAsset.InputActionReferences)
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogISEngineSubsystem_InputActionAssetReferences,
            Verbose,
            GCUtils::Materialize(TStringBuilder<512>())
                << TEXT("Adding new referenced asset '") << GCUtils::String::GetUObjectPathNameSafe(tagToInputActionPair.Value) << TEXT("' by data asset '") << GCUtils::String::GetUObjectPathName(inDataAsset) << TEXT("'.")
                TEXT(" ")
                TEXT("Gameplay tag: '") << tagToInputActionPair.Key.GetTagName() << TEXT("'.")
            );

        const bool didAdd = TryAddReferencedInputAction(tagToInputActionPair.Key, tagToInputActionPair.Value);
        ensure(didAdd);
    }

    return true;
}

bool UISEngineSubsystem_InputActionAssetReferences::TryRemoveReferencedAssetsDataAsset(
    const UISPrimaryDataAsset_InputActionAssetReferences& inDataAsset)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UISEngineSubsystem_InputActionAssetReferences::TryRemoveReferencedAssetsDataAsset);

    // Remove the data asset and all of its added referenced assets.

    GC_LOG_STR_UOBJECT(
        this,
        LogISEngineSubsystem_InputActionAssetReferences,
        Log,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Trying to remove asset refereness data asset '") << GCUtils::String::GetUObjectPathName(inDataAsset) << TEXT("'.")
        );

    const int32 numRemoved = AssetReferencesDataAssetSet.Remove(&inDataAsset);
    if (numRemoved <= 0)
    {
        // Nothing to remove.
        return false;
    }

    ensure(numRemoved == 1);

    for (const TPair<FGameplayTag, TObjectPtr<UInputAction>>& tagToInputActionPair : inDataAsset.InputActionReferences)
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogISEngineSubsystem_InputActionAssetReferences,
            Verbose,
            GCUtils::Materialize(TStringBuilder<512>())
                << TEXT("Removing referenced asset '") << GCUtils::String::GetUObjectPathNameSafe(tagToInputActionPair.Value) << TEXT("' by data asset '") << GCUtils::String::GetUObjectPathName(inDataAsset) << TEXT("'.")
                TEXT(" ")
                TEXT("Gameplay tag: '") << tagToInputActionPair.Key.GetTagName() << TEXT("'.")
            );

        const bool didRemove = TryRemoveReferencedInputAction(tagToInputActionPair.Key) != nullptr;
        ensure(didRemove);
    }

    return true;
}

void UISEngineSubsystem_InputActionAssetReferences::OnAssetManagerCreated()
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UISEngineSubsystem_InputActionAssetReferences::OnAssetManagerCreated);

    // Load the game project's configged references and add them.
    AddGameProjectAssetReferences(UAssetManager::Get());

    GCUtils::Plugin::UseContentFromDependentPlugins(
        TEXT(UE_PLUGIN_NAME),
        GCUtils::Plugin::FPluginRefNativeDelegate::CreateUObject(this, &ThisClass::OnPluginAddContent),
        GCUtils::Plugin::FPluginRefNativeDelegate::CreateUObject(this, &ThisClass::OnPluginRemoveContent)
        );
}

void UISEngineSubsystem_InputActionAssetReferences::AddGameProjectAssetReferences(UAssetManager& inAssetManager)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UISEngineSubsystem_InputActionAssetReferences::AddGameProjectAssetReferences);

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

        TryAddReferencedInputAction(tagToInputActionPair.Key, loadedInputAction);
    }
}

void UISEngineSubsystem_InputActionAssetReferences::OnPluginAddContent(TSharedRef<IPlugin>&& inPlugin)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UISEngineSubsystem_InputActionAssetReferences::OnPluginAddContent);

    GC_LOG_STR_UOBJECT(
        this,
        LogISEngineSubsystem_InputActionAssetReferences,
        Log,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Plugin '") << inPlugin->GetName() << TEXT("' content added. Loading and adding asset references data asset, if any.")
        );

    // No need to have the streamable handle hold our loaded assets in memory as we will already store strong
    // references to them ourselves.
    constexpr bool shouldManageActiveHandle = false;

    TSharedRef<FStreamableHandle> streamableHandle =
        GCUtils::AssetStreaming::LoadSyncChecked<shouldManageActiveHandle>(
            UAssetManager::Get().GetStreamableManager(),
            GetAssetReferenceDataAssetPathForPlugin(inPlugin)
            );

    constexpr bool shouldReportErrors = false;
    const UISPrimaryDataAsset_InputActionAssetReferences* loadedAssetReferenceDataAsset =
        GCUtils::AssetStreaming::GetLoadedAsset<shouldReportErrors, UISPrimaryDataAsset_InputActionAssetReferences>(MoveTemp(streamableHandle));

    if (!loadedAssetReferenceDataAsset)
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogISEngineSubsystem_InputActionAssetReferences,
            Verbose,
            GCUtils::Materialize(TStringBuilder<512>())
                << TEXT("No asset references data asset found for plugin '") << inPlugin->GetName() << TEXT("'.")
                TEXT(" ")
                TEXT("Asset path searched: '") << streamableHandle->GetDebugName() << TEXT("'.");
            );
        return;
    }

    TryAddReferencedAssetsDataAsset(*loadedAssetReferenceDataAsset);
}

void UISEngineSubsystem_InputActionAssetReferences::OnPluginRemoveContent(TSharedRef<IPlugin>&& inPlugin)
{
    TRACE_CPUPROFILER_EVENT_SCOPE(UISEngineSubsystem_InputActionAssetReferences::OnPluginRemoveContent);

    GC_LOG_STR_UOBJECT(
        this,
        LogISEngineSubsystem_InputActionAssetReferences,
        Log,
        GCUtils::Materialize(TStringBuilder<512>())
            << TEXT("Plugin '") << inPlugin->GetName() << TEXT("' content removed. Removing asset references data asset, if any.")
        );

    FSoftObjectPath assetReferenceDataAssetPath = GetAssetReferenceDataAssetPathForPlugin(inPlugin);

    const TObjectPtr<const UISPrimaryDataAsset_InputActionAssetReferences>* foundAssetReferenceDataAsset =
        GCUtils::Set::FindByPredicate<TObjectPtr<const UISPrimaryDataAsset_InputActionAssetReferences>>(
            AssetReferencesDataAssetSet,
            [&assetReferenceDataAssetPath](const TObjectPtr<const UISPrimaryDataAsset_InputActionAssetReferences>& inDataAsset)
            {
                return FSoftObjectPath(inDataAsset) == assetReferenceDataAssetPath;
            }
            );

    if (!foundAssetReferenceDataAsset)
    {
        GC_LOG_STR_UOBJECT(
            this,
            LogISEngineSubsystem_InputActionAssetReferences,
            Verbose,
            GCUtils::Materialize(TStringBuilder<512>())
                << TEXT("No added asset references data asset found for plugin '") << inPlugin->GetName() << TEXT("'.")
            );
        return;
    }

    check(*foundAssetReferenceDataAsset);
    TryRemoveReferencedAssetsDataAsset(**foundAssetReferenceDataAsset);
}
