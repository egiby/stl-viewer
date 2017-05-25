#include "CViewerWindow.h"

#include "Common.h"


CViewerWindow::CViewerWindow( const wchar_t className[], const wchar_t title[] )
	: handle( nullptr ), className( className ), title( title ) {
}

CViewerWindow::~CViewerWindow() {
	delete intersecter;
	delete settings;
}

CViewerWindow* pointerByLong( LONG window ) {
	return reinterpret_cast<CViewerWindow*>(window);
}

LONG longByPointer( CViewerWindow* window ) {
	return reinterpret_cast<LONG>(window);
}

LRESULT CViewerWindow::windowProc( HWND handle, UINT message, WPARAM wParam, LPARAM lParam ) {
	if( message == WM_NCCREATE ) {
		auto params = reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams;
		SetWindowLong( handle, GWL_USERDATA, longByPointer( reinterpret_cast<CViewerWindow*>(params) ) );

		return DefWindowProc( handle, message, wParam, lParam );
	}

	CViewerWindow* window = pointerByLong( GetWindowLong( handle, GWL_USERDATA ) );
	switch( message ) {
		case WM_CREATE:
		{
			logs << "Viewer: WM_CREATE" << std::endl;
			return 0;
		}
		case WM_SIZE:
		{
			logs << "Viewer: WM_SIZE" << std::endl;
			RECT rect;
			GetClientRect( handle, &rect );

			window->Resize( rect.bottom - rect.top, rect.right - rect.left );
			return 0;
		}
		case WM_PAINT:
		{
			logs << "Viewer: WM_PAINT" << std::endl;
			window->OnPaint();
			return 0;
		}
		case WM_DESTROY:
		{
			logs << "Viewer: WM_DESTROY" << std::endl;
			PostQuitMessage( 0 );
			return 0;
		}
		default:
		{
			logs << "Viewer: another message" << std::endl;
			return DefWindowProc( handle, message, wParam, lParam );
		}
	}
}

ATOM CViewerWindow::InitWindowClass( HINSTANCE hInstance ) {
	WNDCLASSEXW wcex = {0};

	wcex.cbSize = sizeof( WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = windowProc;
	wcex.hInstance = hInstance;
	wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);
	wcex.lpszClassName = ViewerClassName;
	wcex.lpszMenuName = ViewerMenuName;
	wcex.hIconSm = LoadIcon( NULL, IDI_APPLICATION );

	return RegisterClassEx( &wcex );
}

Geometry::Point CViewerWindow::calcPixelCenter( uint32_t x, uint32_t y ) const {
	return settings->screen.left_bottom_angle + settings->screen.x_basis * (x + 0.5) +
		settings->screen.y_basis * (y + 0.5);
}

std::unique_ptr<Gdiplus::Graphics> CViewerWindow::fill() {
	std::unique_ptr<Gdiplus::Graphics> ret;
	if( !buffer ) {
		buffer.reset( new Gdiplus::Bitmap( GetWidth(), GetHeight() ) );

		ret.reset( Gdiplus::Graphics::FromImage( buffer.get() ) );
		ret->Clear( Gdiplus::Color( 0, 0, 0 ) );

		for( uint32_t h = 0; h < buffer->GetHeight(); ++h ) {
			for( uint32_t w = 0; w < buffer->GetWidth(); ++w ) {
				NGeometry::Point pixel = calcPixelCenter( w, h );
				NGeometry::Ray ray( settings->eye, pixel - settings->eye );

				auto color = NIntersecter::calcColor( intersecter->intersectAll( ray ), settings, intersecter );
				buffer->SetPixel( w, h, Gdiplus::Color( color.red, color.green, color.blue ) );
			}
		}
	}
	return ret;
}

HWND CViewerWindow::Create( ImageSettings::ImageSettings* _settings ) {
	settings = _settings;
	intersecter = new Calculations::Intersecter( settings );
	logs << "Settings with " << settings->objects.size() << " objects added to CViewerWindow" << std::endl;
	handle = CreateWindowEx(
		0,
		className,
		title,
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		NULL,
		reinterpret_cast<LPVOID>(this)
	);

	if( !handle ) {
		logs << "Create viewver failed" << std::endl;
		PrintLastError();
	}

	return handle;
}

void CViewerWindow::Show( int cmdShow ) const {
	ShowWindow( handle, cmdShow );
	UpdateWindow( handle );
}

uint32_t CViewerWindow::GetHeight() const {
	return settings->screen.x_size;
}

uint32_t CViewerWindow::GetWidth() const {
	return settings->screen.y_size;
}

void CViewerWindow::Resize( uint32_t height, uint32_t width ) {
	logs << "Viewer resize: " << height << ' ' << width << std::endl;
	settings->screen.x_size = height;
	settings->screen.y_size = width;

	buffer.reset();
}

void CViewerWindow::OnDestroy() const {
	DestroyWindow( handle );
}

void CViewerWindow::OnPaint() {
	PAINTSTRUCT paintStruct;
	logs << "Viewer paint" << std::endl;
	HDC hdc = BeginPaint( handle, &paintStruct );

	Gdiplus::Graphics graphics( hdc );
	fill();

	graphics.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );
	graphics.DrawImage( buffer.get(), 200, 0 );

	EndPaint( handle, &paintStruct );
}

