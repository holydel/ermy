#include "../framegraph_interface.h"
#ifdef ERMY_GAPI_D3D12

#include "d3d12_interface.h"
#include <vector>
#include <cassert>
#include "d3d12_swapchain.h"
#include <array>
#include <chrono>

using namespace ermy;

namespace framegraph_interface
{
	uint64_t Signal(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence,
		uint64_t& fenceValue)
	{
		uint64_t fenceValueForSignal = ++fenceValue;
		D3D_CALL(commandQueue->Signal(fence, fenceValueForSignal));

		return fenceValueForSignal;

	}

	void WaitForFenceValue(ID3D12Fence* fence, uint64_t fenceValue, HANDLE fenceEvent,
		std::chrono::milliseconds duration = std::chrono::milliseconds::max())
	{
		if (fence->GetCompletedValue() < fenceValue)
		{
			D3D_CALL(fence->SetEventOnCompletion(fenceValue, fenceEvent));
			::WaitForSingleObject(fenceEvent, static_cast<DWORD>(duration.count()));
		}
	}

	void Flush(ID3D12CommandQueue* commandQueue, ID3D12Fence* fence,
		uint64_t& fenceValue, HANDLE fenceEvent)
	{
		uint64_t fenceValueForSignal = Signal(commandQueue, fence, fenceValue);
		WaitForFenceValue(fence, fenceValueForSignal, fenceEvent);
	}

	//VkSemaphore gFrameGraphSemaphore = VK_NULL_HANDLE;
	u32			gFrameRingCurrent{ 0 };

	struct FrameData
	{
		HANDLE fenceEvent;
		ID3D12Fence* fence;
		UINT64 fenceValue;
		ID3D12GraphicsCommandList* commandList;
		ID3D12CommandAllocator* commandAllocator;

		u64 frameIndex = 0;
	};

	std::vector<FrameData> gFrames;

	void Initialize(u8 numFrames)
	{
		assert(numFrames > 1);

		const uint64_t initialValue = (numFrames - 1);

		gFrames.resize(numFrames);
		for (uint32_t i = 0; i < numFrames; ++i)
		{
			FrameData& fo = gFrames[i];
			D3D_CALL(gD3DDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fo.fence)));


			D3D_CALL(gD3DDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&fo.commandAllocator)));

			D3D_CALL(gD3DDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, fo.commandAllocator, nullptr, IID_PPV_ARGS(&fo.commandList)));

			D3D_CALL(fo.commandList->Close());

			fo.fenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
		}
	}

	void Shutdown()
	{
		/*vkDestroySemaphore(gVKDevice, gFrameGraphSemaphore, nullptr);
		gFrameGraphSemaphore = VK_NULL_HANDLE;
		for (auto& frame : gFrames)
		{
			vkDestroyCommandPool(gVKDevice, frame.cmdPool, nullptr);
			frame.cmdPool = VK_NULL_HANDLE;
			frame.cmdBuffer = VK_NULL_HANDLE;
		}
		gFrames.clear();*/
	}

	void* BeginFrame()
	{
		swapchain::ReInitIfNeeded();

		auto& frame = gFrames[gFrameRingCurrent];

		WaitForFenceValue(frame.fence, frame.fenceValue, frame.fenceEvent);

		frame.commandAllocator->Reset();
		frame.commandList->Reset(frame.commandAllocator, nullptr);

		swapchain::AcquireNextImage();

		return frame.commandList;
	}

	void BeginFinalRenderPass()
	{
		static float gFrameID = 0;
		gFrameID += 1.0f;

		auto& frame = gFrames[gFrameRingCurrent];
		auto& bbResource = GetCurrentBackbufferResourceInfo();
		//acquire the current frame
		//transition resource to clear state


		CD3DX12_RESOURCE_BARRIER barrier[2];
		barrier[0] = CD3DX12_RESOURCE_BARRIER::Transition(
			bbResource.bbResource,
			D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

		frame.commandList->ResourceBarrier(1, barrier);

		float r = sinf((float)gFrameID * 0.005f) * 0.5f + 0.5f;
		float g = cosf((float)gFrameID * 0.006f) * 0.5f + 0.5f;
		float b = sinf((float)gFrameID * 0.05f) * 0.5f + 0.5f;

		FLOAT clearColor[] = { r, g, b, 1.0f };

		frame.commandList->ClearRenderTargetView(bbResource.bbRTV, clearColor, 0, nullptr);
		frame.commandList->OMSetRenderTargets(1, &bbResource.bbRTV, FALSE, nullptr);

	}

	void EndFinalRenderPass()
	{
		auto& frame = gFrames[gFrameRingCurrent];
		auto& bbResource = GetCurrentBackbufferResourceInfo();

		CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
			bbResource.bbResource,
			D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
		frame.commandList->ResourceBarrier(1, &barrier);
	}

	void EndFrame()
	{
		auto& frame = gFrames[gFrameRingCurrent];

		D3D_CALL(frame.commandList->Close());
	}

	void Submit()
	{
		auto& frame = gFrames[gFrameRingCurrent];

		ID3D12CommandList* const commandLists[] = {
	frame.commandList
		};

		gD3DCommandQueue->ExecuteCommandLists(1, commandLists);
	}

	void Present()
	{
		auto& frame = gFrames[gFrameRingCurrent];

		swapchain::Present();

		frame.fenceValue = Signal(gD3DCommandQueue, frame.fence, frame.fenceValue);

		//// Move to the next frame
		gFrameRingCurrent = (gFrameRingCurrent + 1) % swapchain::GetNumFrames();
	}
};
#endif