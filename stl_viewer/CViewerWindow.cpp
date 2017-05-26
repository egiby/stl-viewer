#include "CViewerWindow.h"

#include "Common.h"

#include <Windowsx.h>


CViewerWindow::CViewerWindow( const wchar_t className[], const wchar_t title[] )
	: handle( nullptr ), hStartButton( nullptr ), startButtonStatus( false ), className( className ), title( title ) {
}

CViewerWindow::~CViewerWindow() {
}

CViewerWindow* pointerByLong( LONG window ) {
	return reinterpret_cast<CViewerWindow*>(window);
}

LONG longByPointer( CViewerWindow* window ) {
	return reinterpret_cast<LONG>(window);
}

RECT CViewerWindow::getViewRect() const {
	RECT rect;
	GetWindowRect( handle, &rect );
	rect.left = 200;
	rect.top = 0;
	
	return rect;
}

void CViewerWindow::updateView() const {
	auto rect = getViewRect();
	InvalidateRect( handle, &rect, TRUE );
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
			window->menu = LoadMenu( reinterpret_cast<HINSTANCE>(GetWindowLong( handle, GWL_HINSTANCE )), 
				L"Menu" );
			window->hStartButton = CreateWindowEx( 0,
				L"Button",
				L"Start view",
				WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
				10,
				10,
				100,
				100,
				handle,
				NULL,
				reinterpret_cast<HINSTANCE>(GetWindowLong( handle, GWL_HINSTANCE )),
				NULL );


			for( int i = 0; window->loadFile() && i < 2; ++i ) {}
			
			return 0;
		}
		case WM_SIZE:
		{
			logs << "Viewer: WM_SIZE" << std::endl;
			if( wParam == SIZE_MINIMIZED ) 
				break;

			window->OnResize();

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
			window->OnDestroy();
			PostQuitMessage( 0 );
			return 0;
		}
		case WM_COMMAND:
		{
			switch ( LOWORD( wParam ) ) {
				case BN_CLICKED:
				{
					logs << "BN_CLICKED" << std::endl;
					window->flipStartButton();
					window->painter->SetLeftAngle( 200 );
					window->painter->FlipViewer();
					SetFocus( handle );

					window->OnResize();
					window->updateView();
					break;
				}
				default:
					break;
			}
			return 0;
		}
		case WM_KEYDOWN:
		{
			logs << "WM_KEYDOWN recieved:" << std::endl;
			logs << "VK: " << wParam << std::endl;
			switch( wParam ) {
				case VK_UP:
				{
					window->painter->RotateUp();
					window->updateView();
					break;
				}
				case VK_DOWN:
				{
					window->painter->RotateDown();
					window->updateView();
					break;
				}
				case VK_RIGHT:
				{
					window->painter->RotateRight();
					window->updateView();
					break;
				}
				case VK_LEFT:
				{
					window->painter->RotateLeft();
					window->updateView();
					break;
				}
				case VK_SPACE:
				{
					logs << "VK_SPACE" << std::endl;
					window->painter->MoveUp();
					window->updateView();
					break;
				}
				case VK_RETURN:
				{
					window->painter->MoveDown();
					window->updateView();
					break;
				}
				case VK_ADD:
				{
					window->painter->Compress( 0.9 );
					window->updateView();
					break;
				}
				case VK_SUBTRACT:
				{
					window->painter->Compress( 1 / 0.9 );
					window->updateView();
				}
				default:
				{
					break;
				}
			}
			return 0;
		}
		default:
		{
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

HWND CViewerWindow::Create() {
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

	return handle;
}

void CViewerWindow::Show( int cmdShow ) const {
	ShowWindow( handle, cmdShow );
	UpdateWindow( handle );
}

void CViewerWindow::OnResize() {
	RECT rect = getViewRect();
	
	auto height = rect.bottom - rect.top;
	auto width = rect.right - rect.left;

	logs << "Viewer resize: " << height << ' ' << width << std::endl;
	painter->Resize( height, width );
}

void CViewerWindow::OnDestroy() {
	DestroyWindow( handle );
	DestroyMenu( menu );
}

void CViewerWindow::OnPaint() {
	PAINTSTRUCT paintStruct;
	logs << "Viewer paint" << std::endl;
	
	painter->Paint( handle );
}

void CViewerWindow::OnCreate() {
}

bool CViewerWindow::loadFile() {
	file.lStructSize = sizeof( OPENFILENAME );
	file.hInstance = reinterpret_cast<HINSTANCE>(GetWindowLong( handle, GWL_HINSTANCE ));
	file.lpstrFilter = L"Custom\0*.in;\0STL\0*.stl;\0RT\0*.rt;\0All Files\0*.*\0\0";
	file.lpstrFile = filename;
	file.nMaxFile = 256;
	file.lpstrInitialDir = L".\\";
	file.lpstrDefExt = NULL;

	file.lpstrTitle = L"Open file for viewing";
	file.Flags = OFN_HIDEREADONLY;
	if( !GetOpenFileName( &file ) )
		return true;

	std::wstring extention( filename + file.nFileExtension );

	Parsers::IFileParser * parser = nullptr;
	if ( extention == L"in" ) {
		logs << "Ext: in" << std::endl;
		parser = new Parsers::DefaultParser();
	}
	else if ( extention == L"rt" ) {
		logs << "Ext: rt" << std::endl;
		parser = new Parsers::RTParser();
	}
	else if ( extention == L"stl" ) {
		logs << "Ext: stl" << std::endl;
		parser = new Parsers::STLParser();
	}

	std::ifstream in( filename );
	auto settings = parser->parse( in );

	in.close();
	delete parser;

	painter.reset( new CPainter( settings ) );
	return false;
}

const wchar_t * getStartButtonText(bool status) {
	return status ? L"End view" : L"Start view";
}

void CViewerWindow::flipStartButton() {
	startButtonStatus ^= 1;
	Button_SetText( hStartButton, getStartButtonText( startButtonStatus ) );
}
