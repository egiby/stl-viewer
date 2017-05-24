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
			return 0;
		}
		case WM_SIZE:
		{
			window->OnResize();
			return 0;
		}
		case WM_PAINT:
		{
			window->OnPaint();
			return 0;
		}
		case WM_DESTROY:
		{
			PostQuitMessage( 0 );
			return 0;
		}
		default:
		{
			return DefWindowProc( handle, message, wParam, lParam );
		}
	}
}

ATOM CViewerWindow::InitWindowClass() {
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof( WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = windowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = NULL;
	wcex.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground = CreateSolidBrush( RGB( 255, 255, 255 ) );
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
	//	logs << GetHeight() << ' ' << GetWidth() << std::endl;

	bool need_clear = false;
	std::unique_ptr<Gdiplus::Graphics> ret;
	if( !buffer ) {
		buffer.reset( new Gdiplus::Bitmap( GetWidth(), GetHeight() ) );

		ret.reset( Gdiplus::Graphics::FromImage( buffer.get() ) ) ;
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

bool CViewerWindow::Create( ImageSettings::ImageSettings* _settings ) {
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
		return false;
	}

	return true;
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

void CViewerWindow::OnDestroy() const {
	DestroyWindow( handle );
}

void CViewerWindow::OnPaint() {
	PAINTSTRUCT paintStruct;
	HDC hdc = BeginPaint( handle, &paintStruct );

	Gdiplus::Graphics graphics( hdc );
	fill();

	graphics.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );
	graphics.DrawImage( buffer.get(), 0, 0 );

	EndPaint( handle, &paintStruct );
}

void CViewerWindow::OnResize() {
	RECT rect;
	GetClientRect( handle, &rect );

	settings->screen.x_size = rect.bottom;
	settings->screen.y_size = rect.right;

	buffer.reset();
}
