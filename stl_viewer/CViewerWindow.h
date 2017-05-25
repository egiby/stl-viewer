#pragma once

#include <vector>

#include <Windows.h>
#include "ray_tracing.h"

#include "Common.h"
#include <gdiplus.h>
#include <memory>
#pragma comment (lib,"Gdiplus.lib")

class CViewerWindow {
public:
	explicit CViewerWindow( const wchar_t className[] = ViewerClassName, const wchar_t title[] = ViewerMenuName );
	~CViewerWindow();
	// Register window class
	static ATOM InitWindowClass( HINSTANCE hInstance = NULL );
	HWND Create( ImageSettings::ImageSettings* );
	void Show( int cmdShow ) const;

	uint32_t GetHeight() const;
	uint32_t GetWidth() const;
	void Resize( uint32_t height, uint32_t width );
protected:
	void OnDestroy() const;
	Geometry::Point calcPixelCenter( ui32 x, ui32 y ) const;
	std::unique_ptr<Gdiplus::Graphics> fill();
	void OnPaint();
private:
	HWND handle; // window's handle
	const wchar_t* className;
	const wchar_t* title;

	ImageSettings::ImageSettings* settings;
	Calculations::Intersecter* intersecter;
	
	std::unique_ptr<Gdiplus::Bitmap> buffer;

	static LRESULT CALLBACK windowProc( HWND handle, UINT message, WPARAM wParam, LPARAM lParam );
};
