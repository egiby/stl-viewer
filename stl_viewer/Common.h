#pragma once

#include <Windows.h>
#include <fstream>

inline void PrintLastError() {
	auto error = GetLastError();
	HANDLE stringMessage;
	FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL, error, MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
		reinterpret_cast<LPWSTR>(&stringMessage), 0, NULL );

	MessageBox( NULL, reinterpret_cast<wchar_t*>(stringMessage), NULL, MB_OK );
}

const wchar_t ViewerClassName[] = L"ViewerClass";
const wchar_t ViewerMenuName[] = L"StlViewer";

static std::ofstream logs( "log", std::ios::app );