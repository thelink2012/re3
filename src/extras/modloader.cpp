#include "modloader.h"

#ifdef MODLOADER
#include <windows.h>
#include <cstdio>
#include <cstdint>

#include "modloader/modloader_re3.h"

//
#include "rwcore.h"
extern RwUInt32 gGameState;
extern void PlayMovieInWindow(int, const char*);
//

static HMODULE hModLoader;

static modloader_fInitFromRE3 modloader_InitFromRE3;
static modloader_re3_t modloader_re3_share;
static modloader_re3_addr_table_t re3_addr_table;

void ModLoader_Init()
{
    if(!SetEnvironmentVariableA("MODLOADER_RE3", "1")) {
		fprintf(stderr, "Failed to set MODLOADER_RE3=1 (error %d)\n", GetLastError());
		return;
    }

    ::hModLoader = LoadLibraryA("modloader.dll");
    if(hModLoader == NULL) {
        fprintf(stderr, "Failed to load modloader.dll (error %d)\n", GetLastError());
        return;
    }

    modloader_InitFromRE3 = (modloader_fInitFromRE3) GetProcAddress(::hModLoader, "modloader_InitFromRE3");
    if(modloader_InitFromRE3 == NULL) {
        fprintf(stderr, "modloader_InitFromRE3 not found in modloader.dll\n");
        FreeLibrary(::hModLoader);
        hModLoader = NULL;
        return;
    }

    memset(&modloader_re3_share, 0, sizeof(modloader_re3_share));
    modloader_re3_share.size = sizeof(modloader_re3_share);
    modloader_re3_share.re3_version = 0;
    re3_addr_table.size = sizeof(re3_addr_table);
    modloader_re3_share.re3_addr_table = &re3_addr_table;

    re3_addr_table.p_gGameState = reinterpret_cast<int*>(&gGameState);
    re3_addr_table.PlayMovieInWindow = PlayMovieInWindow; 

    if(!modloader_InitFromRE3(&modloader_re3_share)) {
        FreeLibrary(::hModLoader);
        hModLoader = NULL;
        return;
    }
}

void ModLoader_Shutdown()
{
    if(hModLoader == NULL)
        return;

    modloader_re3_share.Shutdown(&modloader_re3_share);

    FreeLibrary(::hModLoader);
    ::hModLoader = NULL;
}

void ModLoader_Tick()
{
    if(hModLoader == NULL)
        return;

    modloader_re3_share.Tick(&modloader_re3_share);
}

void
ModLoader_PlayMovieInWindow_Logo(int cmdshow, const char *filename)
{
	if(hModLoader == NULL) return;

	if(!modloader_re3_share.callback_table->PlayMovieInWindow_Logo) return modloader_re3_share.re3_addr_table->PlayMovieInWindow(cmdshow, filename);

	return modloader_re3_share.callback_table->PlayMovieInWindow_Logo(cmdshow, filename);
}

void ModLoader_PlayMovieInWindow_GTAtitles(int cmdshow, const char* filename)
{
	if(hModLoader == NULL)
        return;

    if(!modloader_re3_share.callback_table->PlayMovieInWindow_GTAtitles)
		return modloader_re3_share.re3_addr_table->PlayMovieInWindow(cmdshow, filename);

    return modloader_re3_share.callback_table->PlayMovieInWindow_GTAtitles(cmdshow, filename);
}

#endif