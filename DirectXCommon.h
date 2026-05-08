#pragma once
#include <Windows.h>
#include <cstdint>
#include <d3d12.h>
#include <dxgi1_6.h>

// libのリンク
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

class DirectXCommon {
public:
    void Initialize(HWND hwnd);

    void PreDraw();
    void PostDraw();

    ID3D12Device* GetDevice() const { return device_; }
    ID3D12GraphicsCommandList* GetCommandList() const { return commandList_; }

private:
    void CreateDevice();
    void CreateCommand();
    void CreateSwapChain(HWND hwnd);
    void CreateRenderTargetView();
    void CreateFence();

private:
    IDXGIFactory7* dxgiFactory_ = nullptr;
    ID3D12Device* device_ = nullptr;
    IDXGISwapChain4* swapChain_ = nullptr;

    ID3D12CommandQueue* commandQueue_ = nullptr;
    ID3D12CommandAllocator* commandAllocator_ = nullptr;
    ID3D12GraphicsCommandList* commandList_ = nullptr;

    ID3D12DescriptorHeap* rtvDescriptorHeap_ = nullptr;
    ID3D12Resource* swapChainResources_[2] = {};

    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles_[2];

    ID3D12Fence* fence_ = nullptr;
    uint64_t fenceValue_ = 0;
    HANDLE fenceEvent_ = nullptr;
};