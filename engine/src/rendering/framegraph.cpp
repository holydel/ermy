#include "framegraph.h"
#include "framegraph_interface.h"

void framegraph::Initialize(ermy::u8 numFrames)
{
	framegraph_interface::Initialize(numFrames);
}

void framegraph::Shutdown()
{
	framegraph_interface::Shutdown();
}

void framegraph::Process()
{
	framegraph_interface::Process();
}
