#include <Windows.h>
#include <winuser.h>

#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")

#include "Common.h"
#include "CViewerWindow.h"

int WINAPI wWinMain( HINSTANCE instance,
	HINSTANCE prevInstance,
	LPWSTR commandLine,
	int cmdShow ) {

	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;

	Gdiplus::GdiplusStartup( &gdiplusToken, &gdiplusStartupInput, NULL );

	CViewerWindow window;

	if( CViewerWindow::InitWindowClass() == 0 ) {
		PrintLastError();
		return -1;
	}

	Parsers::DefaultParser parser;

	auto settings = parser.parseFile( "settings.in" );

	NGeometry::Point left_bottom_angle = settings->screen.left_bottom_angle;
	NGeometry::Point left_upper = left_bottom_angle + settings->screen.x_basis * settings->screen.x_size;
	NGeometry::Point right_bottom = left_bottom_angle + settings->screen.y_basis * settings->screen.y_size;
	NGeometry::Point right_upper = left_bottom_angle + settings->screen.y_basis * settings->screen.y_size + 
		settings->screen.x_basis * settings->screen.x_size;

//	settings->light_sources.clear();
//	settings->light_sources.push_back( { 4000, left_bottom_angle } );
//	settings->light_sources.push_back( { 4000, left_upper } );
//	settings->light_sources.push_back( { 2000, right_bottom } );
//	settings->light_sources.push_back( { 2000, right_upper } );
//	settings->light_sources.push_back( { 3000, (left_bottom_angle + right_upper) / 2 } );

	if( !window.Create( settings ) ) {
		PrintLastError();
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
