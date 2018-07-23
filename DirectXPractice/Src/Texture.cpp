#include "Texture.h"
#include "d3dx12.h"

using Microsoft::WRL::ComPtr;

/**
* WINC フォーマットから対応するDXGIフォーマットを得る
*
* @param wicFormat WIC フォーマット
*
* @return wicFormat に対応するDXGIフォーマット
*			対応するフォーマットが見つからない場合はDXGI_FORMAT_UNKNOWNを返す
*/
DXGI_FORMAT GetDXGIFormatFromWINCFormat(const WICPixelFormatGUID& wicFormat) {
	static const struct {
		WICPixelFormatGUID guid;
		DXGI_FORMAT format;
	} wicToDxgiList[] = {
		{ GUID_WICPixelFormat128bppRGBAFloat,	DXGI_FORMAT_R32G32B32A32_FLOAT },
	{ GUID_WICPixelFormat64bppRGBAHalf,		DXGI_FORMAT_R16G16B16A16_FLOAT },
	{ GUID_WICPixelFormat64bppRGBA,			DXGI_FORMAT_R16G16B16A16_UNORM },
	{ GUID_WICPixelFormat32bppRGBA,			DXGI_FORMAT_R8G8B8A8_UNORM },
	{ GUID_WICPixelFormat32bppBGRA,			DXGI_FORMAT_B8G8R8A8_UNORM },
	{ GUID_WICPixelFormat32bppBGR,			DXGI_FORMAT_B8G8R8X8_UNORM },
	{ GUID_WICPixelFormat32bppRGBA1010102XR, DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM },
	{ GUID_WICPixelFormat32bppRGBA1010102, DXGI_FORMAT_R10G10B10A2_UNORM },
	{ GUID_WICPixelFormat16bppBGRA5551, DXGI_FORMAT_B5G5R5A1_UNORM },
	{ GUID_WICPixelFormat16bppBGR565, DXGI_FORMAT_B5G6R5_UNORM },
	{ GUID_WICPixelFormat32bppGrayFloat, DXGI_FORMAT_R32_FLOAT },
	{ GUID_WICPixelFormat16bppGrayHalf, DXGI_FORMAT_R16_FLOAT },
	{ GUID_WICPixelFormat16bppGray, DXGI_FORMAT_R16_UNORM },
	{ GUID_WICPixelFormat8bppGray, DXGI_FORMAT_R8_UNORM },
	{ GUID_WICPixelFormat8bppAlpha, DXGI_FORMAT_A8_UNORM },
	};
	for (auto e : wicToDxgiList) {
		if (e.guid == wicFormat) {
			return e.format;
		}
	}
	return DXGI_FORMAT_UNKNOWN;
}

/**
* 任意のWICフォーマットからDXGIフォーマットと互換性のあるWICフォーマットを得る
*
* @param wicFormat WIC フォーマットのGUID
*
* @return DXGIフォーマットと互換性のあるWICフォーマット
*			互換性のあるフォーマットがいつからない場合はGUID_WICPixelFormatDontCareを返す
*/
WICPixelFormatGUID GetDXGICompatibleWICFormat(const WICPixelFormatGUID& wicFormat) {
	static const struct {
		WICPixelFormatGUID guid, compatible;
	} guidToCompatibleList[] = {
		{ GUID_WICPixelFormatBlackWhite, GUID_WICPixelFormat8bppGray },
	{ GUID_WICPixelFormat1bppIndexed, GUID_WICPixelFormat32bppRGBA },
	{ GUID_WICPixelFormat2bppIndexed, GUID_WICPixelFormat32bppRGBA },
	{ GUID_WICPixelFormat4bppIndexed, GUID_WICPixelFormat32bppRGBA },
	{ GUID_WICPixelFormat8bppIndexed, GUID_WICPixelFormat32bppRGBA },
	{ GUID_WICPixelFormat2bppGray, GUID_WICPixelFormat8bppGray },
	{ GUID_WICPixelFormat4bppGray, GUID_WICPixelFormat8bppGray },
	{ GUID_WICPixelFormat16bppGrayFixedPoint, GUID_WICPixelFormat16bppGrayHalf },
	{ GUID_WICPixelFormat32bppGrayFixedPoint, GUID_WICPixelFormat32bppGrayFloat },
	{ GUID_WICPixelFormat16bppBGR555, GUID_WICPixelFormat16bppBGRA5551 },
	{ GUID_WICPixelFormat32bppBGR101010, GUID_WICPixelFormat32bppRGBA1010102 },
	{ GUID_WICPixelFormat24bppBGR, GUID_WICPixelFormat32bppRGBA },
	{ GUID_WICPixelFormat24bppRGB, GUID_WICPixelFormat32bppRGBA },
	{ GUID_WICPixelFormat32bppPBGRA, GUID_WICPixelFormat32bppRGBA },
	{ GUID_WICPixelFormat32bppPRGBA, GUID_WICPixelFormat32bppRGBA },
	{ GUID_WICPixelFormat48bppRGB, GUID_WICPixelFormat64bppRGBA },
	{ GUID_WICPixelFormat48bppBGR, GUID_WICPixelFormat64bppRGBA },
	{ GUID_WICPixelFormat64bppBGRA, GUID_WICPixelFormat64bppRGBA },
	{ GUID_WICPixelFormat64bppPRGBA, GUID_WICPixelFormat64bppRGBA },
	{ GUID_WICPixelFormat64bppPBGRA, GUID_WICPixelFormat64bppRGBA },
	{ GUID_WICPixelFormat48bppRGBFixedPoint, GUID_WICPixelFormat64bppRGBAHalf },
	{ GUID_WICPixelFormat48bppBGRFixedPoint, GUID_WICPixelFormat64bppRGBAHalf },
	{ GUID_WICPixelFormat64bppRGBAFixedPoint, GUID_WICPixelFormat64bppRGBAHalf },
	{ GUID_WICPixelFormat64bppBGRAFixedPoint, GUID_WICPixelFormat64bppRGBAHalf },
	{ GUID_WICPixelFormat64bppRGBFixedPoint, GUID_WICPixelFormat64bppRGBAHalf },
	{ GUID_WICPixelFormat64bppRGBHalf, GUID_WICPixelFormat64bppRGBAHalf },
	{ GUID_WICPixelFormat48bppRGBHalf, GUID_WICPixelFormat64bppRGBAHalf },
	{ GUID_WICPixelFormat128bppPRGBAFloat, GUID_WICPixelFormat128bppRGBAFloat },
	{ GUID_WICPixelFormat128bppRGBFloat, GUID_WICPixelFormat128bppRGBAFloat },
	{ GUID_WICPixelFormat128bppRGBAFixedPoint, GUID_WICPixelFormat128bppRGBAFloat },
	{ GUID_WICPixelFormat128bppRGBFixedPoint, GUID_WICPixelFormat128bppRGBAFloat },
	{ GUID_WICPixelFormat32bppRGBE, GUID_WICPixelFormat128bppRGBAFloat },
	{ GUID_WICPixelFormat32bppCMYK, GUID_WICPixelFormat32bppRGBA },
	{ GUID_WICPixelFormat64bppCMYK, GUID_WICPixelFormat64bppRGBA },
	{ GUID_WICPixelFormat40bppCMYKAlpha, GUID_WICPixelFormat64bppRGBA },
	{ GUID_WICPixelFormat80bppCMYKAlpha, GUID_WICPixelFormat64bppRGBA },
	{ GUID_WICPixelFormat32bppRGB, GUID_WICPixelFormat32bppRGBA },
	{ GUID_WICPixelFormat64bppRGB, GUID_WICPixelFormat64bppRGBA },
	{ GUID_WICPixelFormat64bppPRGBAHalf, GUID_WICPixelFormat64bppRGBAHalf },
	};
	for (auto e : guidToCompatibleList) {
		if (e.guid == wicFormat) {
			return e.compatible;
		}
	}
	return GUID_WICPixelFormatDontCare;
}

/**
* DXGI フォーマットから1ピクセルのバイト数を得る
*
* @param dxgiFormat DXGI フォーマット
*
* @return dxgiFormat に対応するバイト数
*/
int GetDXGIFormatBytesPerPixel(DXGI_FORMAT dxgiFormat) {
	switch (dxgiFormat) {
	case DXGI_FORMAT_R32G32B32A32_FLOAT: return 16;
	case DXGI_FORMAT_R16G16B16A16_FLOAT: return 8;
	case DXGI_FORMAT_R16G16B16A16_UNORM: return 8;
	case DXGI_FORMAT_R8G8B8A8_UNORM: return 4;
	case DXGI_FORMAT_B8G8R8A8_UNORM: return 4;
	case DXGI_FORMAT_B8G8R8X8_UNORM: return 4;
	case DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM: return 4;
	case DXGI_FORMAT_R10G10B10A2_UNORM: return 4;
	case DXGI_FORMAT_B5G5R5A1_UNORM: return 2;
	case DXGI_FORMAT_B5G6R5_UNORM: return 2;
	case DXGI_FORMAT_R32_FLOAT: return 4;
	case DXGI_FORMAT_R16_FLOAT: return 2;
	case DXGI_FORMAT_R16_UNORM: return 2;
	case DXGI_FORMAT_R8_UNORM: return 1;
	case DXGI_FORMAT_A8_UNORM: return 1;
	default: return 1;
	}
}

/**
* テクスチャ読み込み開始
*
* @param heap テクスチャ用のRTVデスクリプタ取得先デスクリプタヒープ
*
* @retval true		読み込みの準備に成功
* @retval false	読み込みの準備に失敗
*/
bool TextureLoader::Begin(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap) {
	descriptorHeap = heap;
	if (FAILED(descriptorHeap->GetDevice(IID_PPV_ARGS(&device)))) {
		return false;
	}
	if (FAILED(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)))) {
		return false;
	}
	if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)))) {
		return false;
	}
	descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	if (FAILED(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&imagingFactory)))) {
		return false;
	}

	return true;
}

/**
* テクスチャ読み込みを終了する
*
* @retval コマンドリストのポインタ
*/
ID3D12GraphicsCommandList* TextureLoader::End() {
	commandList->Close();
	return commandList.Get();
}

/**
* 画像データからテクスチャを作成する
*
* テクスチャ用メモリを確保し、データ転送用命令を内部のコマンドリストに追加する
* 作成したテクスチャに関するデータは texture に格納される
*
* @pre Begin(), End()のあいだで呼ぶこと
*
* @param texture	作成したテクスチャを管理するオブジェクト
* @param index		作成したテクスチャを参照するために使われるRTVデスクリプタのインデックス
* @param desc		テクスチャの詳細情報
* @param data		画像データへのポインタ
* @param name		テクスチャ用リソースに着ける名前（デバッグ名）
*
* @retval	true	作成成功
* @retval	false	作成失敗
*/
bool TextureLoader::Create(Texture& texture, int index, const D3D12_RESOURCE_DESC& desc, const void* data, const wchar_t* name) {
	ComPtr<ID3D12Resource> textureBuffer;
	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&desc,
		D3D12_RESOURCE_STATE_COPY_DEST,
		nullptr,
		IID_PPV_ARGS(&textureBuffer)))) {
		return false;
	}
	if (name) {
		textureBuffer->SetName(name);
	}

	UINT64 textureHeapSize;
	device->GetCopyableFootprints(&desc, 0, 1, 0, nullptr, nullptr, nullptr, &textureHeapSize);
	ComPtr<ID3D12Resource> uploadBuffer;
	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(textureHeapSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadBuffer)))) {
		return false;
	}

	const int bytesPerRow = static_cast<int>(desc.Width * GetDXGIFormatBytesPerPixel(desc.Format));
	D3D12_SUBRESOURCE_DATA subresource = {};
	subresource.pData = data;
	subresource.RowPitch = bytesPerRow;
	subresource.SlicePitch = bytesPerRow * desc.Height;
	if (UpdateSubresources<1>(commandList.Get(), textureBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subresource) == 0) {
		return false;
	}
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE));
	device->CreateShaderResourceView(textureBuffer.Get(), nullptr, CD3DX12_CPU_DESCRIPTOR_HANDLE(descriptorHeap->GetCPUDescriptorHandleForHeapStart(), index, descriptorSize));

	texture.resource = textureBuffer;
	texture.format = desc.Format;
	texture.handle = CD3DX12_GPU_DESCRIPTOR_HANDLE(descriptorHeap->GetGPUDescriptorHandleForHeapStart(), index, descriptorSize);

	uploadHeapList.push_back(uploadBuffer);
	return true;
}


/**
* ファイルからテクスチャを読み込む
*
* @param texture	読み込んだテクスチャを管理するオブジェクト
* @param index		読み込んだテクスチャ用のRTV用デスクリプタのインデックス
* @param filename	テクスチャファイル名
*
* @retval true		読み込み成功
* @retval false	読み込み失敗
*/
bool TextureLoader::LoadFromFile(Texture & texture, int index, const wchar_t * filename) {
	ComPtr<IWICBitmapDecoder> decorder;
	if (FAILED(imagingFactory->CreateDecoderFromFilename(filename, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnLoad, decorder.GetAddressOf()))) {
		return false;
	}
	ComPtr<IWICBitmapFrameDecode> frame;
	if (FAILED(decorder->GetFrame(0, frame.GetAddressOf()))) {
		return false;
	}
	WICPixelFormatGUID wicFormat;
	if (FAILED(frame->GetPixelFormat(&wicFormat))) {
		return false;
	}
	UINT width, height;
	if (FAILED(frame->GetSize(&width, &height))) {
		return false;
	}
	DXGI_FORMAT dxgiFormat = GetDXGIFormatFromWINCFormat(wicFormat);

	bool imageConverted = false;
	ComPtr<IWICFormatConverter> converter;
	if (dxgiFormat == DXGI_FORMAT_UNKNOWN) {
		const WICPixelFormatGUID compatibleFormat = GetDXGICompatibleWICFormat(wicFormat);
		if (compatibleFormat == GUID_WICPixelFormatDontCare) {
			return false;
		}
		dxgiFormat = GetDXGIFormatFromWINCFormat(compatibleFormat);
		if (FAILED(imagingFactory->CreateFormatConverter(converter.GetAddressOf()))) {
			return false;
		}
		BOOL canConvert = FALSE;
		if (FAILED(converter->CanConvert(wicFormat, compatibleFormat, &canConvert))) {
			return false;
		}
		if (!canConvert) {
			return false;
		}
		if (FAILED(converter->Initialize(frame.Get(), compatibleFormat, WICBitmapDitherTypeNone, nullptr, 0, WICBitmapPaletteTypeCustom))) {
			return false;
		}
		imageConverted = true;
	}

	const int bytesPerRow = width * GetDXGIFormatBytesPerPixel(dxgiFormat);
	const int imageSize = bytesPerRow * height;
	std::vector<uint8_t> imageData;
	imageData.resize(imageSize);
	if (imageConverted) {
		if (FAILED(converter->CopyPixels(nullptr, bytesPerRow, imageSize, imageData.data()))) {
			return false;
		}
	}
	else {
		if (FAILED(frame->CopyPixels(nullptr, bytesPerRow, imageSize, imageData.data()))) {
			return false;
		}
	}

	const D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(dxgiFormat, width, height, 1, 1);
	if (!Create(texture, index, desc, imageData.data(), filename)) {
		return false;
	}
	return true;
}
