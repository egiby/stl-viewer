#pragma once

#include <Windows.h>

#include "ray_tracing.h"

#include <gdiplus.h>
#pragma comment (lib,"Gdiplus.lib")

#include <memory>

class CPainter {
public:
	CPainter() = delete;
	explicit CPainter( ImageSettings::ImageSettings* );
	
	void AddSettings( ImageSettings::ImageSettings* );
	void SetLeftAngle( int x );
	void Resize( uint32_t height, uint32_t width );
	void Compress( double coef );
	void FlipViewer();

	void RotateUp();
	void RotateDown();
	void RotateLeft();
	void RotateRight();
	void MoveUp();
	void MoveDown();

	void Paint( HWND handle );

	uint32_t GetHeight() const;
	uint32_t GetWidth() const;
private:
	std::unique_ptr<Gdiplus::Graphics> fill();
	Geometry::Point calcPixelCenter( ui32 x, ui32 y ) const;
	Geometry::Point getCenter() const;
	
	bool isViewerVisible;
	std::unique_ptr<ImageSettings::ImageSettings> settings;
	Geometry::Vector normal() const;
	void recalcEye();
	void rotateByAxis( Geometry::Vector axis );
	void move( double len );

	std::unique_ptr<Calculations::Intersecter> intersecter;

	std::unique_ptr<Gdiplus::Bitmap> buffer;
	int leftAngle;
	static const double EyeDistance;
	static const double RotateAngle;
	static const double Step;

	ImageSettings::Screen initialScreen;
};
