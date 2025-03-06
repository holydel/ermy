#pragma once
#include "xr.h"
#include <ermy_commandlist.h>

namespace xr_interface
{
	void Initialize();
	void Shutdown();
	void CreateSession();

	void WaitFrame();
	void AcquireImage();
	void ReleaseImage();
	void SubmitXRFrame();
	[[nodiscard("Check Should Rendering")]] bool BeginXRFinalRenderPass(ermy::rendering::CommandList& cl);
	void EndXRFinalRenderPass(ermy::rendering::CommandList& cl);
}