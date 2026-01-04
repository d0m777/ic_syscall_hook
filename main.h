#pragma once
#include <Windows.h>
#include <DbgHelp.h>
#include <stdio.h>
#include <cstdint>

#if _DEBUG
#pragma comment(lib,"Dbghelp.lib")
#endif

typedef NTSTATUS( NTAPI* pNtSetInformationProcess )(
	HANDLE ProcessHandle,
	PROCESS_INFORMATION_CLASS ProcessInformationClass,
	PVOID ProcessInformation,
	ULONG ProcessInformationLength
	);

typedef struct _PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION
{
	ULONG Version;
	ULONG Reserved;
	PVOID Callback;
} PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION, * PPROCESS_INSTRUMENTATION_CALLBACK_INFORMATION;

extern "C" VOID instrumentation_callback_thunk( VOID );
extern "C" VOID instrumentation_callback( PCONTEXT ctx );

#define RIP_SANITY_CHECK( Rip, BaseAddress, ModuleSize ) ( Rip > BaseAddress ) && ( Rip < ( BaseAddress + ModuleSize ) )

static PVOID g_ntdll_base;
static DWORD g_ntdll_size;