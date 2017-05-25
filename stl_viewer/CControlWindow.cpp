#include "CControlWindow.h"

CControlWindow::CControlWindow( const wchar_t className[], const wchar_t title[] )
	: handle( nullptr ), className( className ), title( title ) {
}

CControlWindow::~CControlWindow() {
}

ATOM CControlWindow::InitWindowClass() {
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof( WNDCLASSEX );

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = windowProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = NULL;
	wcex.hIcon = LoadIcon( NULL, IDI_APPLICATION );
	wcex.hCursor = LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BACKGROUND);
	wcex.lpszClassName = ControlClassName;
	wcex.lpszMenuName = ControlMenuName;
	wcex.hIconSm = LoadIcon( NULL, IDI_APPLICATION );


	auto childStatus = CViewerWindow::InitWindowClass();

	if ( childStatus == 0 ) {
		PrintLastError();
	}

	auto parentStatus = RegisterClassEx( &wcex );

	return parentStatus ? parentStatus * childStatus != 0 : 0;
}

bool CControlWindow::Create() {
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

void CControlWindow::Show( int cmdShow ) const {
	ShowWindow( handle, cmdShow );
	UpdateWindow( handle );
}

void CControlWindow::OnDestroy() const {
	DestroyWindow( hViewer );
	DestroyWindow( handle );
}

void CControlWindow::OnPaint() {
	CheckViewer();
}

void CControlWindow::OnResize() {
	CheckViewer();
	RECT rect;
	GetWindowRect( handle, &rect );

	MoveWindow( hViewer,
		rect.right,
		0,
		rect.right,
		rect.bottom,
		TRUE );

	viewer->Show( SW_SHOW );
}

void CControlWindow::OnCreate() {
	viewer.reset();
}

void CControlWindow::CheckViewer() {
	if( !viewer ) {
		viewer.reset( new CViewerWindow() );
		Parsers::DefaultParser parser;
		auto settings = parser.parseFile( "settings.in" );
		hViewer = viewer->Create( handle, settings );

		viewer->Show( SW_SHOW );
	}
}


CControlWindow* pointerByLong( LONG window ) {
	return reinterpret_cast<CControlWindow*>(window);
}

LONG longByPointer( CControlWindow* window ) {
	return reinterpret_cast<LONG>(window);
}

LRESULT CControlWindow::windowProc( HWND handle, UINT message, WPARAM wParam, LPARAM lParam ) {
	if( message == WM_NCCREATE ) {
		auto params = reinterpret_cast<CREATESTRUCT*>(lParam)->lpCreateParams;
		SetWindowLong( handle, GWL_USERDATA, longByPointer( reinterpret_cast<CControlWindow*>(params) ) );

		return DefWindowProc( handle, message, wParam, lParam );
	}

	CControlWindow* window = pointerByLong( GetWindowLong( handle, GWL_USERDATA ) );
	switch( message ) {
		case WM_CREATE:
		{
			window->OnCreate();
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