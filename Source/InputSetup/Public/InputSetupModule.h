// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

/**
 *
 */
class FInputSetupModule : public FDefaultModuleImpl
{
public:

    // ~ IModuleInterface overrides.
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    // ~ IModuleInterface overrides.
};
