#include <Windows.h>
#include <cassert>

#include "sdk/CHLClient.h"
#include "sdk/IClientEntityList.h"
#include "sdk/IVEngineClient.h"
#include "sdk/IPanel.h"
#include "sdk/ClientModeShared.h"
#include "sdk/ISurface.h"

typedef PVOID (*CreateInterface)(const char* pName, int* pReturnCode);

HMODULE GetModuleHandleAssert(LPCWSTR module_name) {
    HMODULE result = GetModuleHandle(module_name);
    assert(result);
    return result;
}

CreateInterface GetInterfaceAssert(HMODULE _module) {
    CreateInterface _interface = reinterpret_cast<CreateInterface>(GetProcAddress(_module, "CreateInterface"));
    assert(_interface != nullptr);
    return _interface;
}

ClientModeShared* GetClientModeShared(CHLClient* client) {
    // TODO: This made sense when I wrote it, I forgot why the hell it does now.
    /*
        -> CHLClient VTable
        -> HudProccessInput
        -> + 0x5 to ptr to client mode from opcode mov ecx g_pClientmode
        -> deref to get address of global ptr g_pClientmode
        -> g_pClientmode
    */
    void** table = *(void***)client;
    return **(ClientModeShared***)((uintptr_t)table[10] + 0x5);
}

CHLClient* client;
IClientEntityList* entity_list;
IVEngineClient* engine;
ClientModeShared* client_mode_shared;
IPanel* panel;
ISurface* surface;

typedef void (__thiscall* PaintTraverse)(uintptr_t, unsigned int, bool, bool);
PaintTraverse org_paint_traverse;

unsigned int top_panel = -1;
void __fastcall PaintTraverseHooked(uintptr_t ecx, uintptr_t edx, unsigned int vguiPanel, bool forceRepaint, bool allowForce) {
    org_paint_traverse(ecx, vguiPanel, forceRepaint, allowForce);

    if (top_panel == -1)
    {
        if (strcmp(panel->GetName(vguiPanel), "MatSystemTopPanel")) {
            top_panel = vguiPanel;
        }
    }

    if (vguiPanel == top_panel) {
        surface->DrawSetColor(255, 0, 0, 255);
        surface->DrawFilledRect(20, 20, 50, 50);
    }
}

DWORD WINAPI MyThread(LPVOID lpParam) {
    HMODULE client_module = GetModuleHandle(L"engine.dll");
    // Get interfaces factories 
    CreateInterface client_interface = GetInterfaceAssert(GetModuleHandleAssert(L"client.dll"));
    CreateInterface engine_interface = GetInterfaceAssert(GetModuleHandleAssert(L"engine.dll"));
    CreateInterface vgui2_interface = GetInterfaceAssert(GetModuleHandleAssert(L"vgui2.dll"));
    CreateInterface vguimatsurface_interface = GetInterfaceAssert(GetModuleHandleAssert(L"vguimatsurface.dll"));

    // Get classes
    // TODO: Stop repeating this bruv
    client = reinterpret_cast<CHLClient*>(client_interface("VClient017", nullptr));
    assert(client);

    entity_list = reinterpret_cast<IClientEntityList*>(client_interface("VClientEntityList003", nullptr));
    assert(entity_list);
    
    engine = reinterpret_cast<IVEngineClient*>(engine_interface("VEngineClient013", nullptr));
    assert(engine);

    panel = reinterpret_cast<IPanel*>(vgui2_interface("VGUI_Panel009", nullptr));
    assert(panel);

    surface = reinterpret_cast<ISurface*>(vgui2_interface("VGUI_Surface030", nullptr));
    assert(surface);

    // Get ClientModeShared from CHLClient
    ClientModeShared* client_mode_shared = GetClientModeShared(client);
    assert(client_mode_shared);

    // Hooking
    // TODO: Clean, this method sucks.
    uintptr_t* old_table  = *reinterpret_cast<uintptr_t**>(panel);

    size_t total_functions = 0;
    while (old_table[total_functions]) 
        total_functions++;

    uintptr_t* new_table = new uintptr_t[total_functions];
    memcpy(new_table, old_table, sizeof(uintptr_t) * total_functions);

    org_paint_traverse = reinterpret_cast<PaintTraverse>(old_table[41]);
    new_table[41] = reinterpret_cast<uintptr_t>(PaintTraverseHooked);
 
    *reinterpret_cast<uintptr_t**>(panel) = new_table;

    return 0;
}

BOOL APIENTRY DllMain(
    HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) 
        CreateThread(
            NULL,
            0, 
            MyThread, 
            &hModule, 
            0, 
            NULL
        );
    
    return TRUE;
}

