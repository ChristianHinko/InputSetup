// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "ISEngineSubsystem_InputActionAssetReferences.generated.h"

class UInputAction;
class UISPrimaryDataAsset_InputActionAssetReferences;
class IPlugin;
class UAssetManager;
class UEngine;

DECLARE_MULTICAST_DELEGATE_TwoParams(FISReferencedInputActionNativeDelegate,
    const FGameplayTag& /* inTag */,
    const UInputAction& /* inInputAction */);

/**
 * @brief Subsystem holding references to all input actions which can be retrieved
 *        by gameplay tag. Holds all input actions for the game.
 *
 *        To add references from your game: Configure InputSetup in project settings.
 *        To add references from a plugin: Use `UISPrimaryDataAsset_InputActionAssetReferences`.
 */
UCLASS(Config="ISEngineSubsystem_InputActionAssetReferences")
class INPUTSETUP_API UISEngineSubsystem_InputActionAssetReferences : public UEngineSubsystem
{
    GENERATED_BODY()

public:

    UISEngineSubsystem_InputActionAssetReferences();

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

    static UISEngineSubsystem_InputActionAssetReferences& GetChecked(const UEngine& inEngine);

    static FSoftObjectPath GetAssetReferenceDataAssetPathForPlugin(
        const TSharedRef<const IPlugin>& inPlugin);

public:

    /**
     * @brief Get an input action by gameplay tag.
     */
    const UInputAction* GetInputAction(const FGameplayTag& inTag) const;

    FORCEINLINE const TMap<FGameplayTag, TObjectPtr<const UInputAction>>& GetAllInputActions() const
    {
        return ReferencedInputActions;
    }

protected:

    /**
     * @brief Add an input action asset reference.
     * @return True if successful.
     */
    bool TryAddReferencedInputAction(const FGameplayTag& inTag, const UInputAction* inAsset);

    /**
     * @brief Add an input action asset reference.
     * @return True if successful.
     */
    bool TryAddReferencedInputAction(const FGameplayTag& inTag, const UInputAction& inAsset);

    /**
     * @brief Remove an input action asset reference by its tag.
     * @return The input action removed, if any.
     */
    const UInputAction* TryRemoveReferencedInputAction(const FGameplayTag& inTag);

protected:

    /**
     * @brief Add an asset references data asset, adding all its assets references as referenced assets.
     * @return True if successful.
     */
    bool TryAddReferencedAssetsDataAsset(
        const UISPrimaryDataAsset_InputActionAssetReferences& inDataAsset);

    /**
     * @brief Remove an asset references data asset, removing all its referenced assets.
     * @return True if there was one to remove.
     */
    bool TryRemoveReferencedAssetsDataAsset(
        const UISPrimaryDataAsset_InputActionAssetReferences& inDataAsset);

protected:

    void OnAssetManagerCreated();

protected:

    /**
     * @brief Loads and adds all referenced assets from the game project.
     */
    void AddGameProjectAssetReferences(UAssetManager& inAssetManager);

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
     * @brief Container of all referenced assets.
     * @todo Use `std::reference_wrapper<>` for the input action pointers.
     */
    UPROPERTY(Transient)
    TMap<FGameplayTag, TObjectPtr<const UInputAction>> ReferencedInputActions;

    /**
     * @brief External contributions to our input action references.
     * @todo Use `std::reference_wrapper<>` for the asset pointers.
     */
    UPROPERTY(VisibleDefaultsOnly, Category = "InputSetup", DisplayName = "Asset Reference Data Assets (Read-Only)")
    TSet<TObjectPtr<const UISPrimaryDataAsset_InputActionAssetReferences>> AssetReferencesDataAssetSet;

public:

    /**
     * @brief Delegate broadcasted when a new input action reference is added.
     */
    FISReferencedInputActionNativeDelegate OnInputActionAddedDelegate;

    /**
     * @brief Delegate broadcasted when a existing input action reference is removed.
     */
    FISReferencedInputActionNativeDelegate OnInputActionRemovedDelegate;
};
