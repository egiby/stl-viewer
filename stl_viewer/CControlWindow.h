#pragma once

#include <Windows.h>
#include "Common.h"

#include "CViewerWindow.h"

class CControlWindow {
public:
	explicit CControlWindow( const wchar_t className[] = ControlClassName,
		const wchar_t title[] = ControlMenuName );
	~CControlWindow();
	// Register window class
	static ATOM InitWindowClass();
	bool Create();
	void Show( int cmdShow ) const;

protected:
	void OnDestroy() const;
	void OnPaint();
	void OnResize();
	void OnCreate();
	void CheckViewer();
private:
	HWND handle; // window's handle
	const wchar_t* className;
	const wchar_t* title;
	HWND hViewer;
	std::unique_ptr<CViewerWindow> viewer;

	static LRESULT CALLBACK windowProc( HWND handle, UINT message, WPARAM wParam, LPARAM lParam );
};
