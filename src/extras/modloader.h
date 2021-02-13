#pragma once
#include "config.h"

#ifdef MODLOADER

void ModLoader_Init();
void ModLoader_Shutdown();
void ModLoader_Tick();

void ModLoader_PlayMovieInWindow_Logo(int, const char*);
void ModLoader_PlayMovieInWindow_GTAtitles(int, const char*);

#endif