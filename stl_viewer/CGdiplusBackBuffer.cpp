#include "CGdiplusBackBuffer.h"

CGdiplusBackBuffer::CGdiplusBackBuffer() {
}

CGdiplusBackBuffer::~CGdiplusBackBuffer() {
}

std::unique_ptr<Gdiplus::Graphics> CGdiplusBackBuffer::StartRender() {
	if( !m_bitmap ) {
		m_bitmap.reset( new Gdiplus::Bitmap( m_size.Width, m_size.Height ) );
	}
	const Gdiplus::Color black( 255, 255, 255 );
	std::unique_ptr<Gdiplus::Graphics> ret{Gdiplus::Graphics::FromImage( m_bitmap.get() )};
	ret->Clear( black );
	return ret;
}

void CGdiplusBackBuffer::SetSize( const Gdiplus::Size& size ) {
	if( m_size.Width != size.Width || m_size.Height != size.Height ) {
		m_size = size;
		m_bitmap.reset();
	}
}

Gdiplus::Bitmap* CGdiplusBackBuffer::GetBitmap() const {
	return m_bitmap.get();
}
