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
	HWND Create();
	void Show( int cmdShow ) const;

protected:
	void OnResize();
	void OnDestroy();
	void OnPaint();
	void OnButtons( WPARAM wParam, LPARAM lParam );
	void OnStartButton();
	void OnLoadButton();
	void OnKeydown( WPARAM wParam );
private:
	HWND handle; // window's handle
	HMENU menu;

	bool loadFile();
	OPENFILENAME file;
	wchar_t filename[256];

	void CreateButtons( HWND newHandle );
	HWND hStartButton;
	bool startButtonStatus;
	void flipStartButton();

	HWND hLoadButton;

	const wchar_t* className;
	const wchar_t* title;

	std::unique_ptr<CPainter> painter;
	RECT getViewRect() const;
	void updateView() const;

	static LRESULT CALLBACK windowProc( HWND handle, UINT message, WPARAM wParam, LPARAM lParam );
	static const int LeftAngle;
	static const int StartButtonId;
	static const int LoadButtonId;
};
