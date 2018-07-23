#pragma once

#include <d3d12.h>
#include <dxgiformat.h>
#include <wrl/client.h>
#include <vector>
#include <wincodec.h>

/*
Texture 構造体
*/
struct Texture {
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	DXGI_FORMAT format;
	D3D12_GPU_DESCRIPTOR_HANDLE handle;
};

/*
Texture 生成　読み込み　クラス
*/
class TextureLoader {
public:
	TextureLoader() = default;
	~TextureLoader() = default;
	bool Begin(Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> heap);
	ID3D12GraphicsCommandList* End();
	bool Create(Texture& texture, int index, const D3D12_RESOURCE_DESC& desc, const void* data, const wchar_t* name = nullptr);
	bool LoadFromFile(Texture& texture, int index, const wchar_t* filename);

private:
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	Microsoft::WRL::ComPtr<IWICImagingFactory> imagingFactory;
	UINT descriptorSize;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> uploadHeapList;
};