#pragma once

#include <winrt/base.h>
#include <filesystem>

class Application
{
	winrt::com_ptr<ID3D11Device> d3d_device;
	winrt::com_ptr<ID3D11DeviceContext> d3d_context;
	winrt::com_ptr<IDXGIDevice> dx_device;
	winrt::com_ptr<ID2D1Factory1> d2d_factory;
	winrt::com_ptr<ID2D1Device> d2d_device;
	winrt::com_ptr<ID2D1DeviceContext> d2d_context;
	winrt::com_ptr<IWICImagingFactory2> wic_factory;
	winrt::com_ptr<ID2D1Bitmap> d2d_mark_bitmap;

	int argc;
	char** argv;

public:
	Application(int argc, char** argv);
	void run();

	void process_file(const std::filesystem::path& path) const;
};

