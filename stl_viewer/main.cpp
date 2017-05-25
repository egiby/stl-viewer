#include <Windows.h>
#include <winuser.h>

#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")

#include "Common.h"
#include "CViewerWindow.h"
#include "CControlWindow.h"

int WINAPI wWinMain( HINSTANCE instance,
	HINSTANCE prevInstance,
	LPWSTR commandLine,
	int cmdShow ) {

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;

	Gdiplus::GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, NULL );

	CControlWindow window;

	if( CControlWindow::InitWindowClass() == 0 ) {
		PrintLastError();
		logs << "init failed" << std::endl;
		return -1;
	}

	if( !window.Create() ) {
		PrintLastError();
		logs << "create failed" << std::endl;
		return -1;
	}

	window.Show( cmdShow );

	MSG message;
	BOOL getMessageResult = 0;

	while( (getMessageResult = GetMessage( &message, NULL, 0, 0 )) != 0 ) {
		if( getMessageResult == -1 ) {
			PrintLastError();
			return -1;
		}
		TranslateMessage( &message );
		DispatchMessage( &message );
	}


	Gdiplus::GdiplusShutdown( gdiplusToken );

	return 0;
}
