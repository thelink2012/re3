#include "modloader.h"

#ifdef MODLOADER
#include <windows.h>
#include <cstdio>
#include <cstdint>

#include "modloader/modloader_re3.h"



//
#include "common.h"
#include "Streaming.h"
#include "Directory.h"
#include "CdStream.h"

extern RwUInt32 gGameState;
extern void PlayMovieInWindow(int, const char*);

extern BOOL _gbCdStreamOverlapped;
extern BOOL _gbCdStreamAsync;
extern DWORD _gdwCdStreamFlags;
extern HANDLE gCdStreamSema;
extern Queue gChannelRequestQ;
extern struct CdReadInfo *gpReadInfo;

static bool CDirectory__FindItem4(void *pThis, const char *name, uint32_t *out1, uint32_t *out2)
{
    return reinterpret_cast<CDirectory*>(pThis)->FindItem(name, *out1, *out2);
}
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
    re3_addr_table.ms_aInfoForModel = CStreaming::ms_aInfoForModel;
    re3_addr_table.uMaxResources = NUMSTREAMINFO;
    re3_addr_table.p_gCdStreamFlags = &_gdwCdStreamFlags;
    re3_addr_table.p_gCdStreamSema = &gCdStreamSema;
    re3_addr_table.p_gChannelRequestQ = &gChannelRequestQ;
    re3_addr_table.p_gpReadInfo = (void**) &gpReadInfo;
    re3_addr_table.p_gbCdStreamOverlapped = &_gbCdStreamOverlapped;
    re3_addr_table.p_gbCdStreamAsync = &_gbCdStreamAsync;
    re3_addr_table.p_ms_pStreamingBuffer = (void**) CStreaming::ms_pStreamingBuffer;
    re3_addr_table.p_ms_streamingBufferSize = (uint32_t*) &CStreaming::ms_streamingBufferSize;
    re3_addr_table.CDirectory__FindItem4 = CDirectory__FindItem4;
    re3_addr_table.CStreaming__LoadCdDirectory0 = &CStreaming::LoadCdDirectory;
    re3_addr_table.CStreaming__LoadCdDirectory2 = &CStreaming::LoadCdDirectory;
    re3_addr_table.CdStreamRead = &CdStreamRead;
    re3_addr_table.LoadCdDirectoryUsingCallbacks = &CStreaming::LoadCdDirectoryUsingCallbacks;

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


bool
ModLoader_CdStreamThread()
{
	if(modloader_re3_share.callback_table->CdStreamThread) {
        modloader_re3_share.callback_table->CdStreamThread();
        return true;
    }
    return false; 
}


void*
ModLoader_AcquireNextModelFileHandle()
{
	if(modloader_re3_share.callback_table->AcquireNextModelFileHandle)
		return modloader_re3_share.callback_table->AcquireNextModelFileHandle();
    return INVALID_HANDLE_VALUE;
}

void
ModLoader_LoadCdDirectory()
{
	if(modloader_re3_share.callback_table->LoadCdDirectory0)
        return modloader_re3_share.callback_table->LoadCdDirectory0();
	return modloader_re3_share.re3_addr_table->CStreaming__LoadCdDirectory0();
}

void
ModLoader_FetchCdDirectory(const char *filename, int cd_index)
{
	if(modloader_re3_share.callback_table->FetchCdDirectory)
        return modloader_re3_share.callback_table->FetchCdDirectory(filename, cd_index);
	return modloader_re3_share.re3_addr_table->CStreaming__LoadCdDirectory2(filename, cd_index);
}

void
ModLoader_RegisterNextModelRead(int resource_id)
{
	if(modloader_re3_share.callback_table->RegisterNextModelRead)
        return modloader_re3_share.callback_table->RegisterNextModelRead(resource_id);
}

int32_t
ModLoader_CdStreamRead(int32_t channel, void *buffer, uint32_t offset, uint32_t size)
{
	if(modloader_re3_share.callback_table->CdStreamThread)
        return modloader_re3_share.callback_table->CdStreamRead(channel, buffer, offset, size);
	return modloader_re3_share.re3_addr_table->CdStreamRead(channel, buffer, offset, size);
}

#endif