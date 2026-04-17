#pragma once

#include <Windows.h>

#define KDEXT_64BIT
#include <wdbgexts.h>
#ifndef WDBGAPI
#define WDBGAPI
#endif

#include <DbgEng.h>
#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>

#include "LightHook.h"

#pragma comment(lib, "dbgeng.lib")
#pragma comment(lib, "psapi.lib")

class WdbgExtsHook
{
public:
	static bool install();
	static void uninstall();
	static void invalidate_cache();

private:
	static HRESULT STDAPICALLTYPE Hook_DebugCreate(REFIID InterfaceId, PVOID* Interface);

	static void patch_client(IDebugClient* client);
	static void patch_control(IDebugControl* ctrl);

	static HRESULT STDMETHODCALLTYPE Patched_ClientQI(IDebugClient* self, REFIID riid, void** ppv);
	static HRESULT STDMETHODCALLTYPE Patched_GetWindbgExtensionApis64(
		IDebugControl* self, PWINDBG_EXTENSION_APIS64 Api);

	static ULONG_PTR WDBGAPI Proxy_GetExpression(PCSTR lpExpression);
	static void WDBGAPI Proxy_Ioctl(USHORT IoctlType, PVOID lpvData, ULONG cbSizeOfContext);

	static bool is_cacheable_ioctl(USHORT IoctlType, PVOID lpvData, ULONG cbSizeOfContext);
	static std::string build_ioctl_key(PSYM_DUMP_PARAM pSym);

	struct FieldCacheData
	{
		ULONG size;
		ULONG64 address;
		ULONG TypeId;
		ULONG FieldOffset;
		USHORT BitPosition;
		USHORT BitSize;
		ULONG fPointer;
		ULONG fArray;
		ULONG fStruct;
		ULONG fConstant;
		ULONG fStatic;
	};

	struct IoctlCacheEntry
	{
		std::vector<FieldCacheData> fields;
	};

	static HookInformation s_hook;
	static bool s_installed;

	static void** s_orig_client_vtable;
	static void** s_patched_client_vtable;
	static void** s_orig_control_vtable;
	static void** s_patched_control_vtable;

	static PWINDBG_IOCTL_ROUTINE s_orig_ioctl;
	static PWINDBG_GET_EXPRESSION s_orig_get_expression;

	static std::mutex s_expr_mutex;
	static std::mutex s_ioctl_mutex;
	static std::unordered_map<std::string, ULONG_PTR> s_expr_cache;
	static std::unordered_map<std::string, IoctlCacheEntry> s_ioctl_cache;

	// IDebugControl vtable: GetWindbgExtensionApis64 is at slot 79 (0-indexed)
	// IUnknown(3) + IDebugControl methods up to GetWindbgExtensionApis64
	static constexpr int SLOT_QI = 0;
	static constexpr int SLOT_GET_WINDBG_EXT_APIS64 = 79;
	static constexpr int CLIENT_VTABLE_SIZE = 60;
	static constexpr int CONTROL_VTABLE_SIZE = 100;
};
