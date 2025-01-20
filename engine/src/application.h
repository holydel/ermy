#pragma once
#include "ermy_application.h"

void ErmyApplicationRun();

bool ErmyApplicationStep();
void ErmyApplicationShutdown();
void ErmyApplicationStart();

ermy::Application& GetApplication();