#pragma once

#include <Windows.h>
#include "ray_tracing.h"

#include "Common.h"
#include <gdiplus.h>

class CViewerWindow {
public:
	explicit CViewerWindow( const wchar_t className[] = ViewerClassName, const wchar_t title[] = ViewerMenuName );
	~CViewerWindow();
	// ���������������� ����� ����
	static ATOM InitWindowClass();
	// ������� ��������� ����
	bool Create( ImageSettings::ImageSettings *);
	// �������� ����
	void Show( int cmdShow ) const;
protected:
	void OnDestroy() const;
	Geometry::Point calcPixelCenter( ui32 x, ui32 y ) const;
	void paint();
	void OnPaint();
private:
	HWND handle; // ����� ����
	const wchar_t *className;
	const wchar_t *title;
	Gdiplus::Bitmap *bitmap;
	Gdiplus::CachedBitmap *cBitmap;

	ImageSettings::ImageSettings * settings;
	Calculations::Intersecter * intersecter;

	static LRESULT CALLBACK windowProc( HWND handle, UINT message, WPARAM wParam, LPARAM lParam );
};
