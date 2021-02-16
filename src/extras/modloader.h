#pragma once
#include "config.h"

#ifdef MODLOADER

#include <cstdint>

void ModLoader_Init();
void ModLoader_Shutdown();
void ModLoader_Tick();

void ModLoader_PlayMovieInWindow_Logo(int, const char*);
void ModLoader_PlayMovieInWindow_GTAtitles(int, const char*);

bool ModLoader_CdStreamThread();
void* ModLoader_AcquireNextModelFileHandle();
void ModLoader_LoadCdDirectory();
void ModLoader_FetchCdDirectory(const char*, int);
void ModLoader_RegisterNextModelRead(int);
int32_t ModLoader_CdStreamRead(int32_t channel, void *buffer, uint32_t offset, uint32_t size);

void ModLoader_OnRequestSpecialModel(uint32_t model_id, const char *model_name, uint32_t pos, uint32_t size);

#endif