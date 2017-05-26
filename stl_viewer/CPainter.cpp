#include "CPainter.h"

#include "Common.h"

#include <array>
#define _USE_MATH_DEFINES
#include <math.h>

Geometry::Vector normalized( Geometry::Vector v )
{
	return v / NGeometry::abs( v );
}

void CPainter::FlipViewer()
{
	isViewerVisible ^= 1;
	settings->screen = initialScreen;
	buffer.reset();
}

std::array<Geometry::Vector, 3> getRotateMatrix( Geometry::Vector axis, double angle )
{
	logs << "axis is normal: " << NDouble::isEqual( NGeometry::abs( axis ), 1 ) << std::endl;
	auto matrix = std::array<Geometry::Vector, 3>();
	matrix[0].x = cos( angle ) + (1 - cos( angle )) * axis.x * axis.x;
	matrix[0].y = (1 - cos( angle )) * axis.x * axis.y - sin( angle ) * axis.z;
	matrix[0].z = (1 - cos( angle )) * axis.x * axis.z + sin( angle ) * axis.y;
	matrix[1].x = (1 - cos( angle )) * axis.x * axis.y + sin( angle ) * axis.z;
	matrix[1].y = cos( angle ) + (1 - cos( angle )) * axis.y * axis.y;
	matrix[1].z = (1 - cos( angle )) * axis.z * axis.y - sin( angle ) * axis.x;
	matrix[2].x = (1 - cos( angle )) * axis.x * axis.z - sin( angle ) * axis.y;
	matrix[2].y = (1 - cos( angle )) * axis.z * axis.y + sin( angle ) * axis.x;
	matrix[2].z = cos( angle ) + (1 - cos( angle )) * axis.z * axis.z;

	return matrix;
}

Geometry::Vector multiply( std::array<Geometry::Vector, 3> matrix, Geometry::Vector vector )
{
	return {matrix[0] * vector, matrix[1] * vector, matrix[2] * vector};
}

void CPainter::rotateByAxis( Geometry::Vector axis )
{
	axis = normalized( axis );
	auto rotate = getRotateMatrix( axis, RotateAngle );

	auto center = getCenter();

	auto delta_angle = settings->screen.left_bottom_angle - center;
	auto delta_eye = settings->eye - center;

	delta_angle = multiply( rotate, delta_angle );
	delta_eye = multiply( rotate, delta_eye );
	settings->screen.x_basis = multiply( rotate, settings->screen.x_basis );
	settings->screen.y_basis = multiply( rotate, settings->screen.y_basis );

	settings->screen.left_bottom_angle = center + delta_angle;
	settings->eye = center + delta_eye;

	buffer.reset();
}

void CPainter::move( double len )
{
	Geometry::Vector delta = normal();
	if( delta * (settings->screen.left_bottom_angle - settings->eye) < 0 ) {
		delta = -delta;
	}

	delta = delta * len;

	logs << "Delta: " << delta << std::endl;

	settings->screen.left_bottom_angle = settings->screen.left_bottom_angle + delta;
	settings->eye = settings->eye + delta;
	logs << "left bottom angle: " << settings->screen.left_bottom_angle;
	buffer.reset();
}

void CPainter::RotateUp()
{
	if( !isViewerVisible ) {
		return;
	}

	rotateByAxis( -settings->screen.y_basis );
}

void CPainter::RotateDown()
{
	if( !isViewerVisible ) {
		return;
	}

	rotateByAxis( settings->screen.y_basis );
}

void CPainter::RotateLeft()
{
	if( !isViewerVisible ) {
		return;
	}

	rotateByAxis( settings->screen.x_basis );
}

void CPainter::RotateRight()
{
	if( !isViewerVisible ) {
		return;
	}

	rotateByAxis( -settings->screen.x_basis );
}

CPainter::CPainter( ImageSettings::ImageSettings* newSettings )
	: isViewerVisible( false ), leftAngle( 0 ), initialScreen( newSettings->screen )
{
	AddSettings( newSettings );
}

void CPainter::AddSettings( ImageSettings::ImageSettings* newSettings )
{
	settings.reset( newSettings );
	intersecter.reset( new Calculations::Intersecter( settings.get() ) );
	initialScreen = settings->screen;
}

void CPainter::SetLeftAngle( int x )
{
	leftAngle = x;
}

void CPainter::Resize( uint32_t height, uint32_t width )
{
	auto center = getCenter();
	logs << "Center: " << center << std::endl;
	settings->screen.x_size = height;
	settings->screen.y_size = width;

	buffer.reset();
}

void CPainter::Compress( double coef )
{
	logs << "compress" << std::endl;
	if( !isViewerVisible ) {
		return;
	}
	auto center = getCenter();

	settings->screen.x_basis = settings->screen.x_basis * coef;
	settings->screen.y_basis = settings->screen.y_basis * coef;

	settings->screen.left_bottom_angle = center -
		(settings->screen.x_basis * GetWidth() + settings->screen.y_basis * GetHeight()) / 2;

	buffer.reset();
}

void CPainter::MoveUp()
{
	logs << "Move up" << std::endl;
	if( !isViewerVisible ) {
		return;
	}

	move( Step );
}

void CPainter::MoveDown()
{
	logs << "Move down" << std::endl;
	if( !isViewerVisible ) {
		return;
	}

	move( -Step );
}

void CPainter::Paint( HWND handle )
{
	PAINTSTRUCT paintStruct;
	logs << "Viewer paint" << std::endl;
	HDC hdc = BeginPaint( handle, &paintStruct );

	Gdiplus::Graphics graphics( hdc );
	if( isViewerVisible ) {
		recalcEye();
		logs << "Eye: " << settings->eye << std::endl;
		logs << "Left bottom angle: " << settings->screen.left_bottom_angle << std::endl;
		logs << "x_basis: " << settings->screen.x_basis << std::endl;
		logs << "y_basis: " << settings->screen.y_basis << std::endl;
		fill();
		graphics.SetSmoothingMode( Gdiplus::SmoothingModeHighQuality );
		graphics.DrawImage( buffer.get(), leftAngle, 0 );
	}

	EndPaint( handle, &paintStruct );
}

uint32_t CPainter::GetHeight() const
{
	return settings->screen.y_size;
}

uint32_t CPainter::GetWidth() const
{
	return settings->screen.x_size;
}

std::unique_ptr<Gdiplus::Graphics> CPainter::fill()
{
	std::unique_ptr<Gdiplus::Graphics> ret;
	if( !buffer ) {
		buffer.reset( new Gdiplus::Bitmap( GetHeight(), GetWidth() ) );

		ret.reset( Gdiplus::Graphics::FromImage( buffer.get() ) );
		ret->Clear( Gdiplus::Color( 0, 0, 0 ) );

		for( uint32_t h = 0; h < buffer->GetHeight(); ++h ) {
			for( uint32_t w = 0; w < buffer->GetWidth(); ++w ) {
				NGeometry::Point pixel = calcPixelCenter( h, w );
				NGeometry::Ray ray( settings->eye, pixel - settings->eye );

				auto color = NIntersecter::calcColor( intersecter->intersectAll( ray ), settings.get(), intersecter.get() );
				buffer->SetPixel( w, h, Gdiplus::Color( color.red, color.green, color.blue ) );
			}
		}
	}
	return ret;
}

Geometry::Point CPainter::calcPixelCenter( ui32 x, ui32 y ) const
{
	return settings->screen.left_bottom_angle + settings->screen.x_basis * (x + 0.5) +
		settings->screen.y_basis * (y + 0.5);
}

Geometry::Point CPainter::getCenter() const
{
	return settings->screen.left_bottom_angle +
		(settings->screen.x_basis * GetHeight() + settings->screen.y_basis * GetWidth()) / 2.;
}

const double CPainter::EyeDistance = 200.;
const double CPainter::RotateAngle = M_PI / 16;
const double CPainter::Step = 10.;


Geometry::Vector CPainter::normal() const
{
	return normalized( settings->screen.x_basis ^ settings->screen.y_basis );
}

void CPainter::recalcEye()
{
	settings->eye = getCenter() - normal() * EyeDistance;
}
