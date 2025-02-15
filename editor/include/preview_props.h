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

};