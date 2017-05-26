#include "CViewerWindow.h"

#include "Common.h"

const int CViewerWindow::LeftAngle = 100;
const int CViewerWindow::StartButtonId = 50100;
const int CViewerWindow::LoadButtonId = 12345;

CViewerWindow::CViewerWindow( const wchar_t className[], const wchar_t title[] )
	: handle( nullptr ), hStartButton( nullptr ), startButtonStatus( false ), className( className ), title( title )
{
}

CViewerWindow::~CViewerWindow()
{
}

CViewerWindow* pointerByLong( LONG window )
{
	return reinterpret_cast<CViewerWindow*>(window);
}

LONG longByPointer( CViewerWindow* window )
{
	return reinterpret_cast<LONG>(window);
}

RECT CViewerWindow::getViewRect() const
{
	RECT rect;
	GetWindowRect( handle, &rect );
	rect.left = LeftAngle;
	rect.top = 0;

	return rect;
}

void CViewerWindow::updateView() const
{
	auto rect = getViewRect();
	InvalidateRect( handle, &rect, TRUE );
}

LRESULT CViewerWindow::windowProc( HWND handle, UINT message, WPARAM wParam, LPARAM lParam )
{
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
			window->CreateButtons( handle );
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
			window->OnButtons(wParam, lParam);
			return 0;
		}
		case WM_KEYDOWN:
		{
			logs << "WM_KEYDOWN recieved:" << std::endl;
			logs << "VK: " << wParam << std::endl;
			window->OnKeydown( wParam );

			return 0;
		}
		default:
		{
			return DefWindowProc( handle, message, wParam, lParam );
		}
	}
}

ATOM CViewerWindow::InitWindowClass( HINSTANCE hInstance )
{
	WNDCLASSEXW wcex = {0};

	wcex.cbSize = sizeof( WNDCLASSEX );

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

HWND CViewerWindow::Create()
{
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

void CViewerWindow::Show( int cmdShow ) const
{
	ShowWindow( handle, cmdShow );
	UpdateWindow( handle );
}

void CViewerWindow::OnResize()
{
	if ( !painter ) {
		return;
	}
	RECT rect = getViewRect();

	auto height = rect.bottom - rect.top;
	auto width = rect.right - rect.left;

	logs << "Viewer resize: " << height << ' ' << width << std::endl;
	painter->Resize( height, width );
}

void CViewerWindow::OnDestroy()
{
	DestroyWindow( handle );
	DestroyMenu( menu );
}

void CViewerWindow::OnPaint()
{
	if( !painter ) {
		return;
	}

	PAINTSTRUCT paintStruct;
	logs << "Viewer paint" << std::endl;

	painter->Paint( handle );
}

void CViewerWindow::OnButtons( WPARAM wParam, LPARAM lParam )
{
	switch( HIWORD( wParam ) ) {
		case BN_CLICKED:
		{
			logs << "BN_CLICKED: " << lParam << std::endl;
			switch( LOWORD( wParam ) ) {
				case StartButtonId:
				{
					OnStartButton();
					break;
				}
				case LoadButtonId:
				{
					OnLoadButton();
					break;
				}
				default:
					break;
			}

			break;
		}
		default:
			break;
	}
}

void CViewerWindow::OnStartButton()
{
	if( !painter ) {
		return;
	}

	logs << "StartButton clicked" << std::endl;
	flipStartButton();
	painter->SetLeftAngle( LeftAngle );
	painter->FlipViewer();
	SetFocus( handle );

	OnResize();
	updateView();
}

const wchar_t* getStartButtonText( bool status )
{
	return status ? L"End view" : L"Start view";
}

void CViewerWindow::OnLoadButton()
{
	painter.reset();
	loadFile();
	startButtonStatus = false;
	SetWindowText( hStartButton, getStartButtonText( startButtonStatus ) );
	updateView();
}

void CViewerWindow::OnKeydown( WPARAM wParam )
{
	switch( wParam ) {
		case VK_UP:
		{
			painter->RotateUp();
			break;
		}
		case VK_DOWN:
		{
			painter->RotateDown();
			break;
		}
		case VK_RIGHT:
		{
			painter->RotateRight();
			break;
		}
		case VK_LEFT:
		{
			painter->RotateLeft();
			break;
		}
		case VK_SPACE:
		{
			logs << "VK_SPACE" << std::endl;
			painter->MoveUp();
			break;
		}
		case VK_RETURN:
		{
			painter->MoveDown();
			break;
		}
		case VK_ADD:
		{
			painter->Compress( 0.9 );
			break;
		}
		case VK_SUBTRACT:
		{
			painter->Compress( 1 / 0.9 );
			break;
		}
		default:
		{
			return;
		}
	}
	updateView();
}

bool CViewerWindow::loadFile()
{
	file.lStructSize = sizeof( OPENFILENAME);
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

	Parsers::IFileParser* parser = nullptr;
	if( extention == L"in" ) {
		logs << "Ext: in" << std::endl;
		parser = new Parsers::DefaultParser();
	}
	else if( extention == L"rt" ) {
		logs << "Ext: rt" << std::endl;
		parser = new Parsers::RTParser();
	}
	else if( extention == L"stl" ) {
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

void CViewerWindow::CreateButtons( HWND newHandle )
{
	hStartButton = CreateWindowEx( 0,
		L"Button",
		L"Start view",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		10,
		10,
		90,
		90,
		newHandle,
		reinterpret_cast<HMENU>(StartButtonId),
		reinterpret_cast<HINSTANCE>(GetWindowLong( newHandle, GWL_HINSTANCE )),
		NULL );

	hLoadButton = CreateWindowEx( 0,
		L"Button",
		L"Load new file",
		WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
		10,
		100,
		90,
		90,
		newHandle,
		reinterpret_cast<HMENU>(LoadButtonId),
		reinterpret_cast<HINSTANCE>(GetWindowLong( newHandle, GWL_HINSTANCE )),
		NULL );
}

void CViewerWindow::flipStartButton()
{
	startButtonStatus ^= 1;
	SetWindowText( hStartButton, getStartButtonText( startButtonStatus ) );
}
