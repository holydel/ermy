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
	virtual void MouseZoom(float)
	{

	}
	virtual void MouseDown(float normalizedX, float normalizedY)
	{

	}
	virtual void MouseUp()
	{

	}
	virtual void MouseMove(float normalizedDeltaX, float normalizedDeltaY)
	{

	}
	virtual void ResetView()
	{

	}
};