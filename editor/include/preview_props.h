#pragma once
#include <ermy_commandlist.h>
class PreviewProps
{
public:
	PreviewProps() = default;
	virtual ~PreviewProps() = default;
	virtual void RenderPreview(ermy::rendering::CommandList& cl)
	{
	}
	virtual void RenderStaticPreview(ermy::rendering::CommandList& cl)
	{
	}

	virtual void MouseZoom(float)
	{

	}
	virtual void MouseDown(float normalizedX, float normalized, int button)
	{

	}
	virtual void MouseUp(int button)
	{

	}
	virtual void MouseMove(float normalizedDeltaX, float normalizedDeltaY, int button)
	{

	}
	virtual void ResetView()
	{

	}
};