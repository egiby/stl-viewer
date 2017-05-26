#pragma once

#include <vector>
#include <memory>

#include <Windows.h>

#include "ray_tracing.h"
#include "Common.h"
#include "CPainter.h"

class CViewerWindow {
public:
	explicit CViewerWindow( const wchar_t className[] = ViewerClassName, const wchar_t title[] = ViewerMenuName );
	~CViewerWindow();
	// Register window class
	static ATOM InitWindowClass( HINSTANCE hInstance = NULL );
	HWND Create( ImageSettings::ImageSettings* );
	void Show( int cmdShow ) const;

	void OnResize();
protected:
	void OnDestroy();
	void OnPaint();
	void OnCreate();
private:
	HWND handle; // window's handle
	HMENU menu;
	
	HWND hStartButton;
	bool startButtonStatus;
	void flipStartButton();
	
	const wchar_t* className;
	const wchar_t* title;

	std::unique_ptr<CPainter> painter;
	RECT getViewRect() const;
	void updateView() const;

	static LRESULT CALLBACK windowProc( HWND handle, UINT message, WPARAM wParam, LPARAM lParam );
};
