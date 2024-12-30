// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "ISEngineSubsystem_ObjectReferenceLibrary.generated.h"

class UInputAction;
class UISPrimaryDataAsset_PluginObjectReferenceCollection;
class IPlugin;
class UAssetManager;
class UEngine;

DECLARE_MULTICAST_DELEGATE_TwoParams(FISRegisteredInputActionNativeDelegate,
    const FGameplayTag& /* tag */,
    const UInputAction& /* inputAction */);

/**
 * @brief Subsystem holding references to important objects and/or assets which can be retrieved
 *        via gameplay tag. Holds all input actions for the game.
 *
 *        To add references from your game: Configure InputSetup in project settings.
 *        To add references from a plugin: Use ISPrimaryDataAsset_PluginObjectReferenceCollection.
 */
UCLASS(Config="ISObjectReferenceLibrary")
class INPUTSETUP_API UISEngineSubsystem_ObjectReferenceLibrary : public UEngineSubsystem
{
    GENERATED_BODY()

public:

    UISEngineSubsystem_ObjectReferenceLibrary();

public:

    // ~ UObject overrides.
    virtual void PostInitProperties() override;
    // ~ UObject overrides.

protected:

    // ~ USubsystem overrides.
    virtual void Initialize(FSubsystemCollectionBase& inCollection) override;
    virtual void Deinitialize() override;
    // ~ USubsystem overrides.

public:

    static UISEngineSubsystem_ObjectReferenceLibrary& GetChecked(const UEngine& inEngine);

    static FSoftObjectPath GetPluginObjectReferenceCollectionAssetPath(
        const TSharedRef<const IPlugin>& inPlugin);

public:

    /**
     * @brief Get an input action via gameplay tag.
     */
    const UInputAction* GetInputAction(const FGameplayTag& inTag) const;

    FORCEINLINE const TMap<FGameplayTag, TObjectPtr<const UInputAction>>& GetAllInputActions() const
    {
        return InputActionReferences;
    }

protected:

    /**
     * @brief Register an input action.
     * @return True if successful.
     */
    bool TryRegisterInputAction(const FGameplayTag& inTag, const UInputAction* inAsset);

    /**
     * @brief Register an input action.
     * @return True if successful.
     */
    bool TryRegisterInputAction(const FGameplayTag& inTag, const UInputAction& inAsset);

    /**
     * @brief Unregister an input action by tag.
     * @return The input action removed, if any.
     */
    const UInputAction* UnregisterInputActionByTag(const FGameplayTag& inTag);

protected:

    /**
     * @brief Register an object reference collection.
     * @return True if successful.
     */
    bool TryRegisterObjectReferenceCollection(
        const UISPrimaryDataAsset_PluginObjectReferenceCollection& inAsset);

    /**
     * @brief Unregister an object reference collection.
     * @return True if any unregistered.
     */
    bool TryUnregisterObjectReferenceCollection(
        const UISPrimaryDataAsset_PluginObjectReferenceCollection& inAsset);

protected:

    void OnAssetManagerCreated();

protected:

    /**
     * @brief Loads and registers all input actions from the game project.
     */
    void RegisterGameProjectInputActions(UAssetManager& inAssetManager);

protected:

    void OnPluginAddContent(TSharedRef<IPlugin>&& inPlugin);

    void OnPluginRemoveContent(TSharedRef<IPlugin>&& inPlugin);

protected:

    /**
     * @brief Input action references from the game project added via config (project settings).
     */
    UPROPERTY(EditDefaultsOnly, Config, Category = "InputSetup")
    TMap<FGameplayTag, TSoftObjectPtr<const UInputAction>> GameProjectInputActionReferences;

    /**
     * @brief Container of all registered input action reference.
     * @todo Use `std::reference_wrapper<>` for the input action pointers.
     */
    UPROPERTY(Transient)
    TMap<FGameplayTag, TObjectPtr<const UInputAction>> InputActionReferences;

    /**
     * @brief Contributions from plugins to our object references.
     * @todo Use `std::reference_wrapper<>` for the asset pointers.
     */
    UPROPERTY(VisibleDefaultsOnly, Category = "InputSetup", DisplayName = "Plugin Object Reference Collections (Read-Only)")
    TSet<TObjectPtr<const UISPrimaryDataAsset_PluginObjectReferenceCollection>> PluginObjectReferenceCollections;

public:

    /**
     * @brief Delegate broadcasted when a new input action is registered.
     */
    FISRegisteredInputActionNativeDelegate OnInputActionRegisteredDelegate;

    /**
     * @brief Delegate broadcasted when a existing input action is unregistered.
     */
    FISRegisteredInputActionNativeDelegate OnInputActionUnregisteredDelegate;
};
