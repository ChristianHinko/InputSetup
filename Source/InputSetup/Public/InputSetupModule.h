// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

//#include "InputSetupModule.generated.h" // uncomment if we ever need reflection here



/**
 *
 */
class FInputSetupModule : public FDefaultModuleImpl
{
public:
    //  BEGIN IModuleInterface interface
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    //  END IModuleInterface interface
};
