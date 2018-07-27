#include <Windows.h>
#include "Window/Window.h"
#include "d3dx12.h"
#include <dxgi1_4.h>
#include <wrl/client.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "Texture.h"
#include "Math/Vector/Vector2.h"
#include "Math/Vector/Vector3.h"
#include "Math/Vector/Vector4.h"

#include "Lib/FbxLoader/FbxLoader.h"

#define MODEL_FILE	"Res/chara_mesh.fbx"
#define ANIM_FILE	"Res/chara_animation_all.fbx"

using namespace DirectX;
using Microsoft::WRL::ComPtr;
static const int frameBufferCount = 2;
ComPtr<ID3D12Device> device;
ComPtr<IDXGISwapChain3> swapChain;
ComPtr<ID3D12CommandQueue> commandQueue;
ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
ComPtr<ID3D12Resource> renderTargetList[frameBufferCount];
ComPtr<ID3D12DescriptorHeap> dsvDescriptorHeap;
ComPtr<ID3D12Resource> depthStencilBuffer;
ComPtr<ID3D12DescriptorHeap> csuDescriptorHeap;
ComPtr<ID3D12DescriptorHeap> constantsDescriptorHeap;
ComPtr<ID3D12Resource> constantsResource;
Texture texCircle;
Texture texImage;
ComPtr<ID3D12CommandAllocator> commandAllocator[frameBufferCount];
ComPtr<ID3D12GraphicsCommandList> commandList;
ComPtr<ID3D12Fence> fence;
HANDLE fenceEvent;
UINT64 fenceValue[frameBufferCount];
UINT64 masterFenceValue;
int currentFrameIndex;
int rtvDescriptorSize;
bool warp;

ComPtr<ID3D12RootSignature> rootSignature;
ComPtr<ID3D12PipelineState> pso;
ComPtr<ID3D12Resource> vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
ComPtr<ID3D12Resource> indexBuffer;
D3D12_INDEX_BUFFER_VIEW indexBufferView;

ComPtr<ID3D12Resource> fbxVertexBuffer;
D3D12_VERTEX_BUFFER_VIEW fbxVertexBufferView;
ComPtr<ID3D12Resource> fbxIndexBuffer;
D3D12_INDEX_BUFFER_VIEW fbxIndexBufferView;
std::vector<size_t> baseVertex;
std::vector<size_t> baseIndex;
std::vector<size_t> indexCount;

int fbxIndexCount;
D3D12_VIEWPORT viewport;
D3D12_RECT scissorRect;

bool InitializeD3D();
void FinalizeD3D();
bool Render();
bool WaitForPreviousFrame();
bool WaitForGpu();

bool LoadShader(const wchar_t*, const char*, ID3DBlob**);
bool CreatePSO();
bool CreateVertexBuffer();
bool CreateIndexBuffer();

bool LoadFbxFile(const char* filename);
bool LoadTexture();
bool CreateCircleTexture(TextureLoader& loader);

void DrawTriangle();
void DrawRectangle();

void DrawFbxModel();

//頂点データ型
struct Vertex {
	XMFLOAT3 position;
	XMFLOAT4 color;
	XMFLOAT2 texcoord;
};

mff::Vector3<float> eyePosition(0.0f, 0.0f, -50.0f);
mff::Vector3<float> eyeDirection(0.0f, 0.0f, 1.0f);
mff::Vector2<float> mousePos;
bool mpInit = false;

struct InputLayoutConfig {
	enum FormatType {
		Float,
		Int,
		Uint,
		FormatTypeSize,
	};
	enum Classification {
		Classification_PerVertex,
		Classification_PerInstance,
	};
	InputLayoutConfig() = default;
	InputLayoutConfig(int num) {
		elements.reserve(num);
	}
	void AddElement(const char* semanticName, FormatType fType, size_t count, Classification classification) {
		DXGI_FORMAT format;
		UINT elementSize = count;

		switch (fType)
		{
		case InputLayoutConfig::Float:
			elementSize *= sizeof(float);
			break;
		case InputLayoutConfig::Int:
			elementSize *= sizeof(int);
			break;
		case InputLayoutConfig::Uint:
			elementSize *= sizeof(UINT);
			break;
		default:
			break;
		}

		DXGI_FORMAT formats[FormatTypeSize][4] = {
			{ DXGI_FORMAT_R32_FLOAT, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_R32G32B32_FLOAT, DXGI_FORMAT_R32G32B32A32_FLOAT },
			{ DXGI_FORMAT_R32_SINT, DXGI_FORMAT_R32G32_SINT, DXGI_FORMAT_R32G32B32_SINT, DXGI_FORMAT_R32G32B32A32_SINT },
			{ DXGI_FORMAT_R32_UINT, DXGI_FORMAT_R32G32_UINT, DXGI_FORMAT_R32G32B32_UINT, DXGI_FORMAT_R32G32B32A32_UINT},

		};
		format = formats[fType][count - 1];

		D3D12_INPUT_CLASSIFICATION ic;
		switch (classification)
		{
		case Classification_PerVertex:
			ic = D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
			break;
		case Classification_PerInstance:
			ic = D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA;
		default:
			break;
		}
		elements.push_back({ semanticName, 0U, format, 0U, offset, ic, 0U });
		offset += elementSize;
	}
	const D3D12_INPUT_ELEMENT_DESC* Get() const {
		return elements.data();
	}

	size_t GetElementsNum() const {
		return elements.size();
	}

	std::vector<D3D12_INPUT_ELEMENT_DESC> elements;
	UINT offset = 0;
};
InputLayoutConfig pctLayout(3);
//InputLayoutConfig animationLayout;

class DescriptorList {
public:
	enum Type {
		RTV,
		DSV,
		Resource,
	};
	bool Init(ComPtr<ID3D12Device> device, Type type, size_t maxCount) {
		D3D12_DESCRIPTOR_HEAP_DESC desc = {};
		switch (type) {
		case DescriptorList::RTV:
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
			break;
		case DescriptorList::DSV:
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
			break;
		case DescriptorList::Resource:
			desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		default:
			break;
		}
		desc.NumDescriptors = maxCount;
		desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		if (FAILED(device->CreateDescriptorHeap(&desc, IID_PPV_ARGS(&descriptorHeap)))) {
			return false;
		}
		perDescriptorSize = device->GetDescriptorHandleIncrementSize(desc.Type);
		cpuHandle = descriptorHeap->GetCPUDescriptorHandleForHeapStart();
		gpuHandle = descriptorHeap->GetGPUDescriptorHandleForHeapStart();
		this->device = device;
		this->maxCount = maxCount;
		index = 0;
		return true;
	}

	ID3D12DescriptorHeap* GetHeap() const {
		return descriptorHeap.Get();
	}

	D3D12_GPU_DESCRIPTOR_HANDLE GetHandle(size_t index) {
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(gpuHandle, index, perDescriptorSize);
	}

protected:
	ComPtr<ID3D12Device> device;
	ComPtr<ID3D12DescriptorHeap> descriptorHeap;
	size_t perDescriptorSize;
	size_t index;
	size_t maxCount;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle;
	D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle;
};

class ResourceDescriptorList : public DescriptorList {
public:
	size_t AddConstantDescriptor(D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress, size_t bufferSize) {
		D3D12_CONSTANT_BUFFER_VIEW_DESC desc = {};
		desc.BufferLocation = gpuVirtualAddress;
		desc.SizeInBytes = bufferSize;
		device->CreateConstantBufferView(&desc, CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuHandle, index, perDescriptorSize));
		size_t offset = index;
		++index;
		return offset;
	}

	size_t AddShaderResourceDescriptor(ComPtr<ID3D12Resource> resource) {
		D3D12_SHADER_RESOURCE_VIEW_DESC desc = {};
		desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Texture2D.MipLevels = 1;
		desc.Texture2D.MostDetailedMip = 0;
		device->CreateShaderResourceView(resource.Get(), &desc, CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuHandle, index, perDescriptorSize));
		size_t offset = index;
		++index;
		return offset;
	}
};

class ConstantBuffer {
public:
	bool Init(ComPtr<ID3D12Device> device, size_t dataCount) {
		if (FAILED(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(dataCount * 0x100),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&resource)
		))) {
			return false;
		}
		maxCount = dataCount;
		D3D12_RANGE range = { 0,0 };
		resource->Map(0, &range, (void**)&pResourcePtr);
		gpuVirtualAddress = resource->GetGPUVirtualAddress();
		return true;
	}
	int AddData(void* pData, size_t size) {
		size_t alignmented = size + 0xff;
		size_t count = alignmented / 0x100;
		if ((count + index) > maxCount) {
			return -1;
		}
		size_t offset = index;
		index += count;
		memcpy(pResourcePtr + offset * 0x100, pData, size);
		return offset;
	}

	void Update(int offset, void* pData, size_t size) {
		memcpy(pResourcePtr + offset * 0x100, pData, size);
	}

	D3D12_GPU_VIRTUAL_ADDRESS GetGpuVirtualAddress(int offset) {
		return gpuVirtualAddress + offset * 0x100;
	}
private:
	ComPtr<ID3D12Resource> resource;
	size_t bufferSize;
	size_t maxCount;
	size_t index = 0;
	uint8_t* pResourcePtr;
	D3D12_GPU_VIRTUAL_ADDRESS gpuVirtualAddress;
};

ResourceDescriptorList csuDescriptorList;
const size_t resourceDescriptorCount = 3;
size_t csuDescriptorIndex[resourceDescriptorCount];

ConstantBuffer constantBuffer;
int constantIndex[3];

class RootSignatureFactory {
public:
	RootSignatureFactory(){}
	void AddParameterBlock(size_t blockCount, size_t bindIndex) {
		descRanges.push_back(CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_CBV, blockCount, bindIndex));
	}
	void AddResourceBlock(size_t blockCount, size_t bindIndex) {
		descRanges.push_back(CD3DX12_DESCRIPTOR_RANGE(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, blockCount, bindIndex));
	}
	void AddStaticSampler(size_t bindIndex) {
		staticSamplers.push_back(CD3DX12_STATIC_SAMPLER_DESC(bindIndex));
	}
	bool Create(ComPtr<ID3D12Device> device, ComPtr<ID3D12RootSignature>& rootSignature) {
		CD3DX12_ROOT_PARAMETER rootParameters[1];
		rootParameters[0].InitAsDescriptorTable(descRanges.size(), descRanges.data());

		D3D12_ROOT_SIGNATURE_DESC rsDesc = {
			_countof(rootParameters),
			rootParameters,
			staticSamplers.size(),
			staticSamplers.data(),
			D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT
		};

		ComPtr<ID3DBlob> signatureBlob;
		if (FAILED(D3D12SerializeRootSignature(
			&rsDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signatureBlob, nullptr))) {
			return false;
		}
		if (FAILED(device->CreateRootSignature(
			0,
			signatureBlob->GetBufferPointer(),
			signatureBlob->GetBufferSize(),
			IID_PPV_ARGS(&rootSignature)))) {
			return false;
		}
		return true;
	}
private:
	std::vector<D3D12_DESCRIPTOR_RANGE> descRanges;
	std::vector<CD3DX12_STATIC_SAMPLER_DESC> staticSamplers;
};

class PipelineStateFactory {
public:
	PipelineStateFactory() { desc = {}; }
	bool Create(ComPtr<ID3D12Device> device, bool warp, ComPtr<ID3D12PipelineState>& p) {
		desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		desc.SampleMask = 0xffffffff;
		desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.SampleDesc.Count = 1;
		desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		if (warp) {
			desc.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
		}
		if (FAILED(device->CreateGraphicsPipelineState(&desc, IID_PPV_ARGS(&p)))) {
			return false;
		}
		return true;
	}

	D3D12_GRAPHICS_PIPELINE_STATE_DESC Get(bool warp) {
		desc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		desc.SampleMask = 0xffffffff;
		desc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
		desc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		desc.SampleDesc.Count = 1;
		desc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		desc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		if (warp) {
			desc.Flags = D3D12_PIPELINE_STATE_FLAG_TOOL_DEBUG;
		}
		return desc;
	}
	void SetRootSignature(ID3D12RootSignature* rootSignature) {
		desc.pRootSignature = rootSignature;
	}
	void SetVertexShader(ID3DBlob* vs) {
		desc.VS = {
			vs->GetBufferPointer(),
			vs->GetBufferSize()
		};
	}
	void SetPixelShader(ID3DBlob* ps) {
		desc.PS = {
			ps->GetBufferPointer(),
			ps->GetBufferSize()
		};
	}
	void SetLayout(const InputLayoutConfig& layout) {
		desc.InputLayout.pInputElementDescs = layout.Get();
		desc.InputLayout.NumElements = layout.GetElementsNum();
	}
	void SetRenderTargets(size_t count, DXGI_FORMAT* formats) {
		desc.NumRenderTargets = count;
		for (size_t i = 0; i < count; ++i) {
			desc.RTVFormats[i] = formats[i];
		}
	}
private:
	D3D12_GRAPHICS_PIPELINE_STATE_DESC desc;
};

struct GraphicsPipeline {
	void Use(ComPtr<ID3D12GraphicsCommandList> commandList) {
		commandList->SetGraphicsRootSignature(rootSignature.Get());
		commandList->SetPipelineState(pso.Get());
	}

	ComPtr<ID3D12PipelineState> pso;
	ComPtr<ID3D12RootSignature> rootSignature;
};

GraphicsPipeline gpl;

//頂点データ
const Vertex vertices[] = {
	{ XMFLOAT3(0.0f, 0.5f, 0.5f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.5f, 0.0f) },
	{ XMFLOAT3(0.5f,-0.5f, 0.5f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(-0.5f,-0.5f, 0.5f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },

	{ XMFLOAT3(-0.3f, 0.4f, 0.4f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(0.2f, 0.4f, 0.4f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(0.2f,-0.1f, 0.4f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(-0.3f,-0.1f, 0.4f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
	{ XMFLOAT3(-0.2f, 0.1f, 0.6f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(0.0f, 0.0f) },
	{ XMFLOAT3(0.3f, 0.1f, 0.6f), XMFLOAT4(1.0f, 1.0f, 0.0f, 1.0f), XMFLOAT2(1.0f, 0.0f) },
	{ XMFLOAT3(0.3f,-0.4f, 0.6f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(1.0f, 1.0f) },
	{ XMFLOAT3(-0.2f,-0.4f, 0.6f), XMFLOAT4(1.0f, 0.0f, 1.0f, 1.0f), XMFLOAT2(0.0f, 1.0f) },
};

const uint32_t indices[] = {
	0, 1, 2, 2, 3, 0,
	4, 5, 6, 6, 7, 4,
};

const UINT triangleVertexCount = 3;

//const wchar_t windowClassNmae[] = L"DX12Practice";
const wchar_t windowTitle[] = L"DX12Practice";
const int clientWidth = 800;
const int clientHeight = 600;
//HWND hwnd = nullptr;

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
	CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);

	Window& window = Window::Get();
	if (!window.Init(hInstance, WndProc, clientWidth, clientHeight, windowTitle, nCmdShow)) {
		CoUninitialize();
		return 1;
	}
	if (!InitializeD3D()) {
		CoUninitialize();
		return 1;
	}
	while (!window.WindowDestroyed()) {
		window.UpdateMessage();
		if (!Render()) {
			break;
		}
	}
	FinalizeD3D();
	CoUninitialize();
	return 0;
}

bool InitializeD3D() {
#if defined(_DEBUG)
	// DirectX12のデバッグレイヤーを有効にする
	{
		ComPtr<ID3D12Debug>	debugController;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
			debugController->EnableDebugLayer();
		}
	}
#endif
	//Direct3Dデバイスの選択用ファクトリ
	ComPtr<IDXGIFactory4> dxgiFactory;
	if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)))) {
		return false;
	}
	ComPtr<IDXGIAdapter1> dxgiAdapter;
	int adapterIndex = 0;
	bool adapterFound = false;

	while (dxgiFactory->EnumAdapters1(adapterIndex, &dxgiAdapter) != DXGI_ERROR_NOT_FOUND) {
		DXGI_ADAPTER_DESC1 desc;
		dxgiAdapter->GetDesc1(&desc);
		if (!(desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)) {
			if (SUCCEEDED(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_1, __uuidof(ID3D12Device), nullptr))) {
				adapterFound = true;
				break;
			}
		}
		++adapterIndex;
	}

	//アダプターが見つかっていなかったらワープデバイスの作成を試す
	if (!adapterFound) {
		if (FAILED(dxgiFactory->EnumWarpAdapter(IID_PPV_ARGS(&dxgiAdapter)))) {
			return false;
		}
		warp = true;
	}

	//デバイスの生成
	if (FAILED(D3D12CreateDevice(dxgiAdapter.Get(), D3D_FEATURE_LEVEL_11_1, IID_PPV_ARGS(&device)))) {
		return false;
	}

	//コマンドキューの生成
	const D3D12_COMMAND_QUEUE_DESC cqDesc = {};
	if (FAILED(device->CreateCommandQueue(&cqDesc, IID_PPV_ARGS(&commandQueue)))) {
		return false;
	}

	//スワップチェーンの生成
	DXGI_SWAP_CHAIN_DESC1 scDesc = {};
	scDesc.Width = clientWidth;
	scDesc.Height = clientHeight;
	scDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scDesc.SampleDesc.Count = 1;
	scDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scDesc.BufferCount = frameBufferCount;
	scDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	ComPtr<IDXGISwapChain1> tmpSwapChain;
	if (FAILED(dxgiFactory->CreateSwapChainForHwnd(commandQueue.Get(), Window::Get().GetWindowHandle(), &scDesc, nullptr, nullptr, &tmpSwapChain))) {
		return false;
	}
	tmpSwapChain.As(&swapChain);
	currentFrameIndex = swapChain->GetCurrentBackBufferIndex();

	//デスクリプタヒープの生成
	D3D12_DESCRIPTOR_HEAP_DESC rtvDesc = {};
	rtvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvDesc.NumDescriptors = frameBufferCount;
	rtvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(device->CreateDescriptorHeap(&rtvDesc, IID_PPV_ARGS(&rtvDescriptorHeap)))) {
		return false;
	}
	rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	//レンダーターゲット用デスクリプタの生成
	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (int i = 0; i < frameBufferCount; ++i) {
		if (FAILED(swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargetList[i])))) {
			return false;
		}
		device->CreateRenderTargetView(renderTargetList[i].Get(), nullptr, rtvHandle);
		rtvHandle.ptr += rtvDescriptorSize;
	}

	//デプスステンシル生成
	D3D12_CLEAR_VALUE dsClearValue = {};
	dsClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	dsClearValue.DepthStencil.Depth = 1.0f;
	dsClearValue.DepthStencil.Stencil = 0;
	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(
			DXGI_FORMAT_D32_FLOAT,
			clientWidth, clientHeight,
			1, 0, 1, 0,
			D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&dsClearValue,
		IID_PPV_ARGS(&depthStencilBuffer)
	))) {
		return false;
	}

	D3D12_DESCRIPTOR_HEAP_DESC dsvDesc = {};
	dsvDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvDesc.NumDescriptors = 1;
	dsvDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	if (FAILED(device->CreateDescriptorHeap(&dsvDesc, IID_PPV_ARGS(&dsvDescriptorHeap)))) {
		return false;
	}

	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
	depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
	depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	device->CreateDepthStencilView(
		depthStencilBuffer.Get(),
		&depthStencilDesc,
		dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart()
	);

	//SRV CBV UAV用デスクリプタヒープ生成
	D3D12_DESCRIPTOR_HEAP_DESC csuDesc = {};
	csuDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	csuDesc.NumDescriptors = 1024;
	csuDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	if (FAILED(device->CreateDescriptorHeap(&csuDesc, IID_PPV_ARGS(&csuDescriptorHeap)))) {
		return false;
	}

	csuDescriptorList.Init(device, DescriptorList::Resource, 1024);
	constantBuffer.Init(device, 100);
	XMMATRIX tmp;
	constantIndex[0] = constantBuffer.AddData(&tmp, sizeof(tmp));
	constantIndex[1] = constantBuffer.AddData(&tmp, sizeof(tmp));
	constantIndex[2] = constantBuffer.AddData(&tmp, sizeof(tmp));

	for (int i = 0; i < resourceDescriptorCount; ++i) {
		csuDescriptorIndex[i] = csuDescriptorList.AddConstantDescriptor(constantBuffer.GetGpuVirtualAddress(constantIndex[i]), 0x100);
	}

	//コマンドアロケーターの生成
	for (int i = 0; i < frameBufferCount; ++i) {
		HRESULT hr = device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator[i]));
		if (FAILED(hr)) {
			return false;
		}
	}

	//コマンドリストの生成
	if (FAILED(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator[currentFrameIndex].Get(), nullptr, IID_PPV_ARGS(&commandList)))) {
		return false;
	}
	if (FAILED(commandList->Close())) {
		return false;
	}

	//フェンスとフェンスイベントの生成
	if (FAILED(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)))) {
		return false;
	}
	for (int i = 0; i < frameBufferCount; ++i) {
		fenceValue[i] = 0;
	}
	masterFenceValue = 1;
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (!fenceValue) {
		return false;
	}

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = clientWidth;
	viewport.Height = clientHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	scissorRect.left = 0;
	scissorRect.top = 0;
	scissorRect.right = clientWidth;
	scissorRect.bottom = clientHeight;

	if (!CreatePSO()) {
		return false;
	}

	if (!CreateVertexBuffer()) {
		return false;
	}

	if (!CreateIndexBuffer()) {
		return false;
	}

	if (!LoadTexture()) {
		return false;
	}

	if (!LoadFbxFile(MODEL_FILE)) {
		return false;
	}
	return true;
}

void FinalizeD3D() {
	WaitForGpu();
	CloseHandle(fenceEvent);
}

bool Render() {
	if (!WaitForPreviousFrame()) {
		return false;
	}

	if (FAILED(commandAllocator[currentFrameIndex]->Reset())) {
		return false;
	}
	if (FAILED(commandList->Reset(commandAllocator[currentFrameIndex].Get(), nullptr))) {
		return false;
	}

	XMMATRIX matViewprojection;
	const XMVECTOR eyePos = { eyePosition.x, eyePosition.y, eyePosition.z, 0.0f };
	mff::Vector3<float> eyedir = eyePosition + eyeDirection;
	const XMVECTOR eyeCenter = { eyedir.x, eyedir.y, eyedir.z, 0.0f };
	const XMVECTOR eyeUp = { 0.0f, 1.0f, 0.0f, 0.0f };
	XMMATRIX matView = XMMatrixLookAtLH(eyePos, eyeCenter, eyeUp);
	matViewprojection = matView;
	XMMATRIX matProj = XMMatrixPerspectiveFovLH(3.1515926535 * 0.5f, (float)clientWidth / (float)clientHeight, 0.1f, 1000.0f);
	matViewprojection *= matProj;
	XMFLOAT4X4 mat;
	XMStoreFloat4x4(&mat, (matViewprojection));
	XMFLOAT4X4 view;
	XMStoreFloat4x4(&view, XMMatrixTranspose(matView));
	XMFLOAT4X4 projection;
	XMStoreFloat4x4(&projection, XMMatrixTranspose(matProj));
	XMFLOAT4X4 world;
	mff::Vector3<float> position;
	XMStoreFloat4x4(&world, XMMatrixTranspose(XMMatrixTranslation(position.x, position.y, position.z)));
	{
		void* pConstantResrouce;
		D3D12_RANGE range = { 0,0 };
		//constantsResource->Map(0, &range, &pConstantResrouce);
		XMFLOAT4X4 matrixes[] = {
			view,
			projection,
		};
		/*	memcpy(pConstantResrouce, &view, sizeof(view));
			memcpy((char*)pConstantResrouce + 0x100, &projection, sizeof(projection));
			constantsResource->Unmap(0, nullptr);*/
		constantBuffer.Update(constantIndex[0], &view, sizeof(view));
		constantBuffer.Update(constantIndex[1], &projection, sizeof(projection));
		constantBuffer.Update(constantIndex[2], &world, sizeof(world));
	}

	commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			renderTargetList[currentFrameIndex].Get(),
			D3D12_RESOURCE_STATE_PRESENT,
			D3D12_RESOURCE_STATE_RENDER_TARGET)
	);

	D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	rtvHandle.ptr += rtvDescriptorSize * currentFrameIndex;
	commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);
	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	ID3D12DescriptorHeap* heapList[] = {
		csuDescriptorList.GetHeap(),
	};
	commandList->SetDescriptorHeaps(_countof(heapList), heapList);

	//描画
	gpl.Use(commandList);
	//commandList->SetGraphicsRootSignature(rootSignature.Get());
	//commandList->SetPipelineState(pso.Get());
	//commandList->SetGraphicsRootSignature(rootSignature.Get());
	//commandList->SetGraphicsRootDescriptorTable(0, texCircle.handle);
	commandList->SetGraphicsRootDescriptorTable(0, csuDescriptorList.GetHandle(csuDescriptorIndex[0]));
	//commandList->SetGraphicsRoot32BitConstants(2, 16, &mat, 0);
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->DrawInstanced(triangleVertexCount, 1, 0, 0);

	//commandList->IASetIndexBuffer(&indexBufferView);
	//commandList->DrawIndexedInstanced(_countof(indices), 1, 0, triangleVertexCount, 0);

	DrawFbxModel();

	commandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(
			renderTargetList[currentFrameIndex].Get(),
			D3D12_RESOURCE_STATE_RENDER_TARGET,
			D3D12_RESOURCE_STATE_PRESENT)
	);

	//コマンド登録終了
	if (FAILED(commandList->Close())) {
		return false;
	}

	//コマンド実行
	ID3D12CommandList* ppCommandList[] = { commandList.Get() };
	commandQueue->ExecuteCommandLists(_countof(ppCommandList), ppCommandList);

	//フレームバッファ切り替え
	if (FAILED(swapChain->Present(1, 0))) {
		return false;
	}

	fenceValue[currentFrameIndex] = masterFenceValue;
	if (FAILED(commandQueue->Signal(fence.Get(), fenceValue[currentFrameIndex]))) {
		return false;
	}
	++masterFenceValue;

	return true;
}

bool WaitForPreviousFrame() {
	if (fenceValue[currentFrameIndex] && fence->GetCompletedValue() < fenceValue[currentFrameIndex]) {
		if (FAILED(fence->SetEventOnCompletion(fenceValue[currentFrameIndex], fenceEvent))) {
			return false;
		}
		WaitForSingleObject(fenceEvent, INFINITE);
	}
	currentFrameIndex = swapChain->GetCurrentBackBufferIndex();
	return true;
}

bool WaitForGpu() {
	const UINT64 currentFenceValue = masterFenceValue;
	if (FAILED(commandQueue->Signal(fence.Get(), currentFenceValue))) {
		return false;
	}
	++masterFenceValue;

	if (FAILED(fence->SetEventOnCompletion(currentFenceValue, fenceEvent))) {
		return false;
	}
	WaitForSingleObject(fenceEvent, INFINITE);
	return true;
}

bool LoadShader(const wchar_t* filename, const char* target, ID3DBlob** blob) {
	ComPtr<ID3DBlob> errorBlob;
	if (FAILED(D3DCompileFromFile(filename, nullptr, nullptr, "main", target, D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION, 0, blob, &errorBlob))) {
		if (errorBlob) {
			OutputDebugStringA(static_cast<char*>(errorBlob->GetBufferPointer()));
		}
		return false;
	}
	return true;
}

bool CreatePipelineSteteObject(
	const InputLayoutConfig& layout, 
	const wchar_t* vsFile,
	const wchar_t* psFile,
	GraphicsPipeline& gpl
	) {
	//シェーダコンパイル
	ComPtr<ID3DBlob> vertexShaderBlob;
	if (!LoadShader(L"Res/VertexShader.hlsl", "vs_5_0", &vertexShaderBlob)) {
		return false;
	}
	ComPtr<ID3DBlob> pixelShaderBlob;
	if (!LoadShader(L"Res/PixelShader.hlsl", "ps_5_0", &pixelShaderBlob)) {
		return false;
	}

	//RootSignatureの生成
	//→シェーダとリソースのリンクのさせ方を定義する
	RootSignatureFactory rsFactory;
	rsFactory.AddParameterBlock(3, 0);
	if (!rsFactory.Create(device, gpl.rootSignature)) {
		return false;
	}

	//PipelineStateObjectの生成
	//→シェーダの描画パイプラインに関する設定を保持するオブジェクト
	PipelineStateFactory factory;
	factory.SetLayout(layout);
	factory.SetVertexShader(vertexShaderBlob.Get());
	factory.SetPixelShader(pixelShaderBlob.Get());
	DXGI_FORMAT formats[] = {
		DXGI_FORMAT_R8G8B8A8_UNORM,
	};
	factory.SetRenderTargets(1, formats);
	factory.SetRootSignature(gpl.rootSignature.Get());
	if (factory.Create(device, warp, gpl.pso)) {
		return false;
	}
	return true;
}

bool CreatePSO() {
	//頂点パラメータのレイアウト設定
	pctLayout.AddElement("POSITION", InputLayoutConfig::FormatType::Float, 3, InputLayoutConfig::Classification::Classification_PerVertex);
	pctLayout.AddElement("COLOR", InputLayoutConfig::FormatType::Float, 4, InputLayoutConfig::Classification::Classification_PerVertex);
	pctLayout.AddElement("TEXCOORD", InputLayoutConfig::FormatType::Float, 2, InputLayoutConfig::Classification::Classification_PerVertex);

	//シェーダコンパイル
	ComPtr<ID3DBlob> vertexShaderBlob;
	if (!LoadShader(L"Res/VertexShader.hlsl", "vs_5_0", &vertexShaderBlob)) {
		return false;
	}
	ComPtr<ID3DBlob> pixelShaderBlob;
	if (!LoadShader(L"Res/PixelShader.hlsl", "ps_5_0", &pixelShaderBlob)) {
		return false;
	}

	//RootSignatureの生成
	//→シェーダとリソースのリンクのさせ方を定義する
	RootSignatureFactory rsFactory;
	rsFactory.AddParameterBlock(3, 0);
	if (!rsFactory.Create(device, gpl.rootSignature)) {
		return false;
	}

	//PipelineStateObjectの生成
	//→シェーダの描画パイプラインに関する設定を保持するオブジェクト
	PipelineStateFactory factory;
	factory.SetLayout(pctLayout);
	factory.SetVertexShader(vertexShaderBlob.Get());
	factory.SetPixelShader(pixelShaderBlob.Get());
	DXGI_FORMAT formats[] = {
		DXGI_FORMAT_R8G8B8A8_UNORM,
	};
	factory.SetRenderTargets(1, formats);
	factory.SetRootSignature(gpl.rootSignature.Get());
	if (!factory.Create(device, warp, gpl.pso)) {
		return false;
	}
	return true;
}

bool CreateVertexBuffer() {
	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(vertices)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&vertexBuffer)
	))) {
		return false;
	}
	vertexBuffer->SetName(L"Vertex buffer");

	void* pVertexDataBegin;
	const D3D12_RANGE readRange = { 0,0 };
	if (FAILED(vertexBuffer->Map(0, &readRange, &pVertexDataBegin))) {
		return false;
	}
	memcpy(pVertexDataBegin, vertices, sizeof(vertices));
	vertexBuffer->Unmap(0, nullptr);

	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = sizeof(vertices);
	return true;
}

bool CreateIndexBuffer() {
	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(indices)),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&indexBuffer)
	))) {
		return false;
	}
	indexBuffer->SetName(L"Index buffer");

	void* pIndexDataBegin;
	const D3D12_RANGE readRange = { 0,0 };
	if (FAILED(indexBuffer->Map(0, &readRange, &pIndexDataBegin))) {
		return false;
	}
	memcpy(pIndexDataBegin, indices, sizeof(indices));
	indexBuffer->Unmap(0, nullptr);

	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
	indexBufferView.SizeInBytes = sizeof(indices);
	return true;
}

void DrawTriangle() {
	commandList->SetPipelineState(pso.Get());
	commandList->SetGraphicsRootSignature(rootSignature.Get());
	commandList->SetGraphicsRootDescriptorTable(0, texCircle.handle);
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRect);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->DrawInstanced(triangleVertexCount, 1, 0, 0);
}

void DrawRectangle() {
	commandList->SetGraphicsRootDescriptorTable(0, texImage.handle);
	commandList->IASetIndexBuffer(&indexBufferView);
	commandList->DrawIndexedInstanced(_countof(indices), 1, 0, triangleVertexCount, 0);
}

void DrawFbxModel() {
	commandList->IASetVertexBuffers(0, 1, &fbxVertexBufferView);
	commandList->IASetIndexBuffer(&fbxIndexBufferView);
	for (int i = 0; i < baseIndex.size(); ++i) {
		commandList->DrawIndexedInstanced(indexCount[i], 1, baseIndex[i], baseVertex[i], 0);
	}
}

bool LoadTexture() {
	TextureLoader loader;
	if (!loader.Begin(csuDescriptorHeap)) {
		return false;
	}
	if (!CreateCircleTexture(loader)) {
		return false;
	}
	if (!loader.LoadFromFile(texImage, 1, L"Res/chara2_bm2.png")) {
		return false;
	}
	ID3D12CommandList* ppCommandLists[] = { loader.End() };
	commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
	WaitForGpu();
	return true;
}

bool CreateCircleTexture(TextureLoader& loader) {
	const D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, 256, 256, 1, 1);
	std::vector<uint8_t> image;
	image.resize(static_cast<size_t>(desc.Width * desc.Height) * 4);
	uint8_t* p = image.data();
	for (float y = 0; y < desc.Height; ++y) {
		const float fy = (y / (desc.Height - 1) * 2.0f) - 1.0f;
		for (float x = 0; x < desc.Width; ++x) {
			const float fx = (x / (desc.Width - 1) * 2.0f) - 1.0f;
			const float distance = std::sqrt(fx * fx + fy * fy);
			const float t = 0.02f / std::abs(0.1f - std::fmod(distance, 0.2f));
			const uint8_t col = t < 1.0f ? static_cast<uint8_t>(t * 255.0f) : 255;
			p[0] = p[1] = p[2] = col;
			p[3] = 255;
			p += 4;
		}
	}
	return loader.Create(texCircle, 0, desc, image.data(), L"texCircle");
}

bool LoadFbxFile(const char* filename) {
	FbxLoader::Loader loader;
	if (!loader.Initialize(MODEL_FILE)) {
		return false;
	}
	//loader.SetAxisType(FbxLoader::AxisType::LeftHand);
	std::vector<FbxLoader::StaticMesh> meshes;
	loader.LoadStaticMesh(meshes);

	std::vector<Vertex> vertices;
	std::vector<uint32_t> indices;
	int indexStart = 0;
	for (size_t i = 0; i < meshes.size(); ++i) {
		for (size_t m = 0; m < meshes[i].materials.size(); ++m) {
			baseVertex.push_back(vertices.size());
			baseIndex.push_back(indices.size());
			auto& material = meshes[i].materials[m];

			for (size_t vindex = 0; vindex < material.verteces.size(); ++vindex) {
				auto position = material.verteces[vindex].position;
				vertices.push_back({ {position.x,position.y, position.z},{1.0f,1.0f,1.0f,1.0f},{0.0f,0.0f} });
			}
			for (size_t j = 0; j < material.indeces.size(); ++j) {
				indices.push_back(material.indeces[j]);
			}
			indexCount.push_back(indices.size() - indexStart);
			indexStart = indices.size();
		}
	}

	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(Vertex) * 1024 * 1000),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&fbxVertexBuffer)
	))) {
		return false;
	}

	fbxVertexBuffer->SetName(L"Fbx Vertex Buffer");

	void* pVertexDataBegin;
	const D3D12_RANGE readRange = { 0,0 };
	if (FAILED(fbxVertexBuffer->Map(0, &readRange, &pVertexDataBegin))) {
		return false;
	}
	size_t bufferSize = sizeof(Vertex) * vertices.size();
	memcpy(pVertexDataBegin, vertices.data(), bufferSize);
	fbxVertexBuffer->Unmap(0, nullptr);

	fbxVertexBufferView.BufferLocation = fbxVertexBuffer->GetGPUVirtualAddress();
	fbxVertexBufferView.StrideInBytes = sizeof(Vertex);
	fbxVertexBufferView.SizeInBytes = bufferSize;

	if (FAILED(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(uint32_t) * 1024 * 1000 * 3),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&fbxIndexBuffer)
	))) {
		return false;
	}
	fbxIndexBuffer->SetName(L"Fbx Index buffer");

	void* pIndexDataBegin;
	{
		const D3D12_RANGE readRange = { 0,0 };
		if (FAILED(fbxIndexBuffer->Map(0, &readRange, &pIndexDataBegin))) {
			return false;
		}
	}
	fbxIndexCount = indices.size();

	memcpy(pIndexDataBegin, indices.data(), sizeof(uint32_t) * indices.size());
	fbxIndexBuffer->Unmap(0, nullptr);

	fbxIndexBufferView.BufferLocation = fbxIndexBuffer->GetGPUVirtualAddress();
	fbxIndexBufferView.Format = DXGI_FORMAT_R32_UINT;
	fbxIndexBufferView.SizeInBytes = sizeof(uint32_t) * indices.size();

	return true;
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
	switch (msg) {
	case WM_MOUSEMOVE:
		if (!mpInit) {
			POINT pos;
			GetCursorPos(&pos);
			mousePos.x = pos.x;
			mousePos.y = pos.y;
			mpInit = true;
		}
		else {
			POINT pos;
			GetCursorPos(&pos);
			mff::Vector2<float> delta = mff::Vector2<float>(pos.x, pos.y) - mousePos;
			mousePos = mff::Vector2<float>(pos.x, pos.y);
			float rate = 0.025;
			float theta = rate * delta.x;
			eyeDirection.x = eyeDirection.x * cosf(theta) + eyeDirection.z * sinf(theta);
			eyeDirection.z = eyeDirection.x * -sinf(theta) + eyeDirection.z * cosf(theta);
		}
		return 0;
	case WM_KEYDOWN:
	{
		if (wparam == VK_ESCAPE) {
			DestroyWindow(hwnd);
		}
		float speed = 5.0f;
		switch (wparam)
		{
		case 'W':
			eyePosition += eyeDirection * speed;
			break;
		case 'S':
			eyePosition -= eyeDirection * speed;;
			break;
		case 'A':
			eyePosition -= mff::Vector3<float>(eyeDirection.z, eyeDirection.y, -eyeDirection.x) * speed;;
			break;
		case 'D':
			eyePosition += mff::Vector3<float>(eyeDirection.z, eyeDirection.y, -eyeDirection.x) * speed;;
			break;
		case 'E':
			eyePosition += mff::Vector3<float>(0.0f, 1.0f, 0.0f) * speed;;
			break;
		case 'Q':
			eyePosition -= mff::Vector3<float>(0.0f, 1.0f, 0.0f) * speed;;
		default:
			break;
		}
		return 0;
	}
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	default:
		break;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}
