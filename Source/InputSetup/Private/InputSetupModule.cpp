// Fill out your copyright notice in the Description page of Project Settings.

#include "InputSetupModule.h"

void FInputSetupModule::StartupModule()
{
    IModuleInterface::StartupModule();
}

void FInputSetupModule::ShutdownModule()
{
    IModuleInterface::ShutdownModule();
}

IMPLEMENT_MODULE(FInputSetupModule, InputSetup);
