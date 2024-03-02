// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/EngineSubsystem.h"

#include "ISEngineSubsystem_ObjectReferenceLibrary.generated.h"


class UInputAction;
class UISPrimaryDataAsset_PluginObjectReferenceCollection;
class IPlugin;



DECLARE_MULTICAST_DELEGATE_OneParam(FISInputActionReferenceDelegate, const TPair<PREPROCESSOR_COMMA_SEPARATED(FGameplayTag, TWeakObjectPtr<const UInputAction>)>&);


/**
 * Subsystem holding references to important objects and/or assets which can be retrieved via Gameplay Tag.
 *
 * Holds all InputActions for the game.
 *
 * To add references from your game: Configure InputSetup in project settings.
 * To add references from a plugin: Use ISPrimaryDataAsset_PluginObjectReferenceCollection.
 */
UCLASS(Config="ISObjectReferenceLibrary")
class INPUTSETUP_API UISEngineSubsystem_ObjectReferenceLibrary : public UEngineSubsystem
{
    GENERATED_BODY()

protected:
    /** Contributions from plugins to our object references. */
    UPROPERTY(VisibleDefaultsOnly, DisplayName="Plugin Object Reference Collections (Read-Only)")
        TSet<TObjectPtr<const UISPrimaryDataAsset_PluginObjectReferenceCollection>> PluginObjectReferenceCollections;


    /** InputAction references from the game project added via config (project settings). */
    UPROPERTY(EditDefaultsOnly, Config, Category="InputAction")
        TMap<FGameplayTag, TSoftObjectPtr<const UInputAction>> GameProjectInputActionReferences;

    /**
     * Combined map of InputAction references from the game project and the plugins.
     */
    UPROPERTY(Transient)
        TMap<FGameplayTag, TWeakObjectPtr<const UInputAction>> InputActionReferences;

public:
    UISEngineSubsystem_ObjectReferenceLibrary();

    //  BEGIN UObject interface
    virtual void PostInitProperties() override;
    //  END UObject interface

    //  BEGIN USubsystem interface
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;
    //  END USubsystem interface

    /**
     * Get an InputAction via gameplay tag.
     */
    UFUNCTION(BlueprintPure, Category="InputAction")
        const UInputAction* GetInputAction(const FGameplayTag& InGameplayTag) const;

    const TMap<FGameplayTag, TWeakObjectPtr<const UInputAction>>& GetAllInputActions() const { return InputActionReferences; }

    static FSoftObjectPath GetPluginObjectReferenceCollectionAssetPath(const IPlugin& InPlugin);

    FISInputActionReferenceDelegate OnInputActionAdded;
    FISInputActionReferenceDelegate OnInputActionRemoved;

protected:
    virtual void OnAssetManagerCreated();

    virtual void OnPluginAddContent(const IPlugin& InPlugin);
    virtual void OnPluginRemoveContent(const IPlugin& InPlugin);
};
