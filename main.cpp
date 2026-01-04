#include "main.h"

void instrumentation_callback( PCONTEXT ctx )
{
	auto process_tab = reinterpret_cast< ULONG_PTR >( NtCurrentTeb( ) );

	ctx->Rip = *reinterpret_cast< DWORD64* >( process_tab + 0x02D8 ); // teb->InstrumentationCallbackPreviousPc
	ctx->Rsp = *reinterpret_cast< DWORD64* >( process_tab + 0x02E0 ); // teb->InstrumentationCallbackPreviousSp
	ctx->Rcx = ctx->R10;

	auto ntdll_base = reinterpret_cast< ULONG_PTR >( InterlockedCompareExchangePointer( &g_ntdll_base, NULL, NULL ) );
	auto ntdlll_size = InterlockedCompareExchange( &g_ntdll_size, NULL, NULL );

	if ( RIP_SANITY_CHECK( ctx->Rip, ntdll_base, ntdlll_size ) )
		printf( "[+] Function RIP -> 0x%llx\n", ctx->Rip );

	RtlRestoreContext( ctx, NULL );
}

BOOL WINAPI DllMain( HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved )
{
	if ( fdwReason == DLL_PROCESS_ATTACH )
	{
		g_ntdll_base = GetModuleHandle( L"ntdll.dll" );
		auto dos_header = reinterpret_cast< PIMAGE_DOS_HEADER >( g_ntdll_base );
		auto nt_headers = reinterpret_cast< PIMAGE_NT_HEADERS >( reinterpret_cast< std::uint64_t >( g_ntdll_base ) + dos_header->e_lfanew );
		g_ntdll_size = nt_headers->OptionalHeader.SizeOfImage;

		AllocConsole( );
		FILE* stream;
		freopen_s( &stream, "CONOUT$", "w", stdout );
		printf( "[+] ntdll.dll Base Address: 0x%lu\n", g_ntdll_base );

		PROCESS_INSTRUMENTATION_CALLBACK_INFORMATION nirvana;
		nirvana.Callback = ( PVOID )instrumentation_callback_thunk;
		nirvana.Reserved = 0;
		nirvana.Version = 0;

		pNtSetInformationProcess NtSetInformationProcess = ( pNtSetInformationProcess )GetProcAddress( ( HMODULE )g_ntdll_base, "NtSetInformationProcess" );
		NtSetInformationProcess( GetCurrentProcess( ), ( PROCESS_INFORMATION_CLASS )40, &nirvana, sizeof( nirvana ) );
	}

	return TRUE;
}