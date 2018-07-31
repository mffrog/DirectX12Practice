#pragma once
#include <Windows.h>
#include "../d3dx12.h"
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

class Graphics {
public:
	static Graphics& GetInstance();
	bool Initialize(HWND hwnd,int width, int height);
	void Finalize();

private:
	Graphics() = default;
	Graphics(const Graphics&) = delete;
	Graphics& operator=(const Graphics&) = delete;

	static const int FrameBufferCount = 2;
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<ID3D12CommandList> commandList;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator[FrameBufferCount];

	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	HANDLE fenceEvent;
	UINT64 fenceValue[FrameBufferCount];
	UINT64 masterFenceValue;

	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargetList[FrameBufferCount];
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	int rtvDescriptorSize;

	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;

	int currentFrameIndex;
	bool warp;
};