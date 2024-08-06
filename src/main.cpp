#include <iostream>
#include <OpenImageIO/imageio.h>
#include <wrl/client.h>
#include <wincodec.h>
#include <comdef.h>

using namespace OIIO;
using namespace Microsoft::WRL;

void ConvertEXRtoJXR(const std::string& inputFilename, const std::string& outputFilename) {
	// Read EXR file using OpenImageIO
	std::unique_ptr<ImageInput> in = ImageInput::open(inputFilename);
	if (!in) {
		std::cerr << "Could not open EXR file: " << geterror() << std::endl;
		CoUninitialize();
		return;
	}

	const ImageSpec& spec = in->spec();
	const TypeDesc exrFormat = spec.channel_bytes() == 2 ? TypeDesc::HALF : TypeDesc::FLOAT;
	const int exrComponents = spec.nchannels;
	std::vector<unsigned char> pixels(spec.image_bytes());
	in->read_image(TypeDesc::HALF, pixels.data());
	in->close();
	
	// Convert to WIC format
	ComPtr<IWICImagingFactory> factory = NULL;
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, &factory);
	if (FAILED(hr)) {
		std::cerr << "Failed to create WIC Imaging Factory" << std::endl;
		return;
	}

	ComPtr<IWICStream> piStream = NULL;
	hr = factory->CreateStream(&piStream);
	if (FAILED(hr)) {
		std::cerr << "Failed to create WIC Stream" << std::endl;
		return;
	}

	hr = piStream->InitializeFromFilename(std::wstring(outputFilename.begin(), outputFilename.end()).c_str(), GENERIC_WRITE);
	if(FAILED(hr)) {
		std::cerr << "Failed to initialize Stream from filename." << std::endl;
		return;
	}

	// Save as JXR file
	ComPtr<IWICBitmapEncoder> encoder;
	hr = factory->CreateEncoder(GUID_ContainerFormatWmp, nullptr, &encoder);
	if (FAILED(hr)) {
		std::cerr << "Failed to create WIC Encoder" << std::endl;
		return;
	}

	hr = encoder->Initialize(piStream.Get(), WICBitmapEncoderNoCache);
	if (FAILED(hr)) {
		std::cerr << "Failed to initialize WIC Encoder" << std::endl;
		return;
	}

	ComPtr<IWICBitmapFrameEncode> frame = NULL;
	ComPtr<IPropertyBag2> pPropertybag = NULL;
	hr = encoder->CreateNewFrame(&frame, &pPropertybag);
	if (FAILED(hr)) {
		std::cerr << "Failed to create new frame" << std::endl;
		return;
	}

	hr = frame->Initialize(pPropertybag.Get());
	if (FAILED(hr)) {
		std::cerr << "Failed to initialize frame" << std::endl;
		return;
	}

	hr = frame->SetSize(spec.width, spec.height);
	if (FAILED(hr)) {
		std::cerr << "Failed to set frame size" << std::endl;
		return;
	}

	WICPixelFormatGUID pixelFormat;
	if(exrFormat == TypeDesc::HALF) {
		pixelFormat = exrComponents == 3 ? GUID_WICPixelFormat48bppRGBHalf : GUID_WICPixelFormat64bppRGBHalf;
	} else {
		pixelFormat = exrComponents == 3 ? GUID_WICPixelFormat96bppRGBFloat : GUID_WICPixelFormat128bppRGBAFloat;
	}
	WICPixelFormatGUID intendedFormat = pixelFormat;
	hr = frame->SetPixelFormat(&pixelFormat);
	if (FAILED(hr)) {
		std::cerr << "Failed to set pixel format" << std::endl;
		return;
	}

    hr = IsEqualGUID(pixelFormat, intendedFormat) ? S_OK : E_FAIL;
    if(FAILED(hr)) {
        std::cerr << "Failed pixel format not supported." << std::endl;
		return;
    }

	hr = frame->WritePixels(spec.height, ((UINT) pixels.size()) / spec.height, (UINT) pixels.size(), pixels.data());
	if (FAILED(hr)) {
		std::cerr << "Failed to write pixels" << std::endl;
		CoUninitialize();
		return;
	}

	hr = frame->Commit();
	if (FAILED(hr)) {
		std::cerr << "Failed to commit frame" << std::endl;
		CoUninitialize();
		return;
	}

	hr = encoder->Commit();
	if (FAILED(hr)) {
		std::cerr << "Failed to commit encoder" << std::endl;
		CoUninitialize();
		return;
	}

	
}

int main(int argc, char* argv[]) {
	if (argc != 3) {
		std::cerr << "Usage: " << argv[0] << " <input.exr> <output.jxr>" << std::endl;
		return 1;
	}

	std::string inputFilename(argv[1]);
	std::string outputFilename(argv[2]);

	HRESULT hr = CoInitialize(nullptr);
	if (FAILED(hr)) {
		std::cerr << "Failed to initialize COM" << std::endl;
		return 1;
	}

	ConvertEXRtoJXR(inputFilename, outputFilename);
	CoUninitialize();  
	return 0;
}