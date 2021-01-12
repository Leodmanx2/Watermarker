#include "pch.h"
#include "Application.h"

using winrt::com_ptr;
using winrt::check_hresult;

Application::Application(int argc, char** argv) : argc(argc), argv(argv)
{
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_0,
		D3D_FEATURE_LEVEL_12_1,
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0,
		D3D_FEATURE_LEVEL_9_3,
		D3D_FEATURE_LEVEL_9_2,
		D3D_FEATURE_LEVEL_9_1
	};

	D3D11_CREATE_DEVICE_FLAG flags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#ifdef DEBUG
	flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL d3d_feature_level{};
	check_hresult(D3D11CreateDevice(nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		flags,
		featureLevels,
		ARRAYSIZE(featureLevels),
		D3D11_SDK_VERSION,
		d3d_device.put(),
		&d3d_feature_level,
		d3d_context.put()));

	d3d_device->QueryInterface(__uuidof(IDXGIDevice), dx_device.put_void());
	check_hresult(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, d2d_factory.put()));
	check_hresult(d2d_factory->CreateDevice(dx_device.get(), d2d_device.put()));
	check_hresult(d2d_device->CreateDeviceContext(
		D2D1_DEVICE_CONTEXT_OPTIONS_ENABLE_MULTITHREADED_OPTIMIZATIONS,
		d2d_context.put()));

	check_hresult(CoCreateInstance(CLSID_WICImagingFactory2,
		nullptr,
		CLSCTX_INPROC_SERVER,
		__uuidof(IWICImagingFactory2),
		wic_factory.put_void()));

	com_ptr<IWICBitmapDecoder> wic_mark_decoder;
	check_hresult(wic_factory->CreateDecoderFromFilename(L"mark.png",
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		wic_mark_decoder.put()));

	com_ptr<IWICBitmapFrameDecode> wic_mark_frame;
	check_hresult(wic_mark_decoder->GetFrame(0, wic_mark_frame.put()));

	com_ptr<IWICFormatConverter> wic_mark_converter;
	check_hresult(wic_factory->CreateFormatConverter(wic_mark_converter.put()));
	check_hresult(wic_mark_converter->Initialize(wic_mark_frame.get(),
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeErrorDiffusion,
		nullptr,
		0.0,
		WICBitmapPaletteTypeMedianCut));

	check_hresult(d2d_context->CreateBitmapFromWicBitmap(wic_mark_converter.get(), d2d_mark_bitmap.put()));
}

void Application::run()
{
	for (int i = 1; i < argc; ++i) {
		if (std::filesystem::is_regular_file(argv[i])) {
			process_file(argv[i]);
		}
		else {
			std::clog << "skipping " << argv[i] << " -- not a file\n";
		}
	}
}

void Application::process_file(const std::filesystem::path& path) const
{
	const std::wstring src_path = path.wstring();

	com_ptr<IWICBitmapDecoder> wic_src_decoder;
	check_hresult(wic_factory->CreateDecoderFromFilename(src_path.c_str(),
		nullptr,
		GENERIC_READ,
		WICDecodeMetadataCacheOnDemand,
		wic_src_decoder.put()));

	com_ptr<IWICBitmapFrameDecode> wic_src_frame;
	check_hresult(wic_src_decoder->GetFrame(0, wic_src_frame.put()));

	com_ptr<IWICFormatConverter> wic_src_converter;
	check_hresult(wic_factory->CreateFormatConverter(wic_src_converter.put()));
	check_hresult(wic_src_converter->Initialize(wic_src_frame.get(),
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeErrorDiffusion,
		nullptr,
		0.0,
		WICBitmapPaletteTypeMedianCut));

	com_ptr<ID2D1Bitmap> d2d_src_bitmap;
	check_hresult(d2d_context->CreateBitmapFromWicBitmap(wic_src_converter.get(), d2d_src_bitmap.put()));

	auto [src_width, src_height] = d2d_src_bitmap->GetPixelSize();
	const bool   landscape = src_width >= src_height ? true : false;
	double scale = landscape ? 2048.0 / src_width : 2048.0 / src_height;

	com_ptr<ID2D1Effect> d2d_blend_effect;
	check_hresult(d2d_context->CreateEffect(CLSID_D2D1Blend, d2d_blend_effect.put()));
	d2d_blend_effect->SetValue(D2D1_BLEND_PROP_MODE, D2D1_BLEND_MODE_OVERLAY);

	com_ptr<ID2D1Effect> d2d_scale_effect;
	if (scale < 1.0) {
		check_hresult(d2d_context->CreateEffect(CLSID_D2D1Scale, d2d_scale_effect.put()));
		d2d_scale_effect->SetInput(0, d2d_src_bitmap.get());
		d2d_scale_effect->SetValue<D2D1_VECTOR_2F>(
			D2D1_SCALE_PROP_SCALE,
			{ static_cast<FLOAT>(scale), static_cast<FLOAT>(scale) });
		check_hresult(d2d_scale_effect->SetValue(D2D1_SCALE_PROP_INTERPOLATION_MODE,
			D2D1_SCALE_INTERPOLATION_MODE_CUBIC));
		d2d_blend_effect->SetInputEffect(0, d2d_scale_effect.get());

	}
	else {
		d2d_blend_effect->SetInput(0, d2d_src_bitmap.get());
		scale = 1.0;
	}

	auto [mark_width, mark_height] = d2d_mark_bitmap->GetPixelSize();

	com_ptr<ID2D1Effect> d2d_tile_effect;
	check_hresult(d2d_context->CreateEffect(CLSID_D2D1Tile, d2d_tile_effect.put()));
	check_hresult(d2d_tile_effect->SetValue(D2D1_TILE_PROP_RECT,
		D2D1::RectF(0.0f, 0.0f, static_cast<float>(mark_width), static_cast<float>(mark_height))));
	d2d_tile_effect->SetInput(0, d2d_mark_bitmap.get());

	d2d_blend_effect->SetInputEffect(1, d2d_tile_effect.get());

	const UINT             dst_width = static_cast<UINT>(src_width * scale);
	const UINT             dst_height = static_cast<UINT>(src_height * scale);
	D2D_SIZE_U             dst_size{ dst_width, dst_height };

	D2D1_BITMAP_PROPERTIES1 d2d_dst_bitmap_properties{};
	d2d_dst_bitmap_properties.pixelFormat = d2d_src_bitmap->GetPixelFormat();
	d2d_src_bitmap->GetDpi(&d2d_dst_bitmap_properties.dpiX,
		&d2d_dst_bitmap_properties.dpiY);
	d2d_dst_bitmap_properties.bitmapOptions = D2D1_BITMAP_OPTIONS_TARGET;

	com_ptr<ID2D1Bitmap1> d2d_dst_bitmap;
	check_hresult(d2d_context->CreateBitmap(
		dst_size, nullptr, dst_width * 4, d2d_dst_bitmap_properties, d2d_dst_bitmap.put()));

	d2d_context->SetTarget(d2d_dst_bitmap.get());
	d2d_context->BeginDraw();
	d2d_context->DrawImage(d2d_blend_effect.get());
	check_hresult(d2d_context->EndDraw());

	com_ptr<IWICBitmapEncoder> wic_bitmap_encoder;
	check_hresult(wic_factory->CreateEncoder(GUID_ContainerFormatJpeg, nullptr, wic_bitmap_encoder.put()));

	std::filesystem::path newpath(path);
	newpath.replace_filename(path.stem() += "[watermarked].jpg");
	const std::wstring dst_path = newpath.wstring();
	com_ptr<IWICStream> wic_encode_stream;
	check_hresult(wic_factory->CreateStream(wic_encode_stream.put()));
	check_hresult(wic_encode_stream->InitializeFromFilename(dst_path.c_str(), GENERIC_WRITE));

	check_hresult(wic_bitmap_encoder->Initialize(wic_encode_stream.get(), WICBitmapEncoderNoCache));

	com_ptr<IWICBitmapFrameEncode> wic_encode_frame;
	com_ptr<IPropertyBag2>         wic_encode_properties;
	check_hresult(wic_bitmap_encoder->CreateNewFrame(wic_encode_frame.put(), wic_encode_properties.put()));

	PROPBAG2 option{};
	wchar_t option_name[] = L"ImageQuality";
	option.pstrName = option_name;
	VARIANT value;
	VariantInit(&value);
	value.vt = VT_R4;
	value.fltVal = 0.9f;
	check_hresult(wic_encode_properties->Write(1, &option, &value));

	check_hresult(wic_encode_frame->Initialize(wic_encode_properties.get()));
	check_hresult(wic_encode_frame->SetSize(dst_width, dst_height));

	WICPixelFormatGUID wic_encode_format = GUID_WICPixelFormat24bppBGR;
	check_hresult(wic_encode_frame->SetPixelFormat(&wic_encode_format));

	com_ptr<IWICImageEncoder> wic_image_encoder;
	check_hresult(wic_factory->CreateImageEncoder(d2d_device.get(), wic_image_encoder.put()));

	wic_image_encoder->WriteFrame(d2d_dst_bitmap.get(), wic_encode_frame.get(), nullptr);

	check_hresult(wic_encode_frame->Commit());
	check_hresult(wic_bitmap_encoder->Commit());
	check_hresult(wic_encode_stream->Commit(STGC_DEFAULT));
}
