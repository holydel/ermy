#pragma once

namespace ermy
{
	namespace profiler
	{
		void BeginScope(const char* name);
		void EndScope();
		void NextFrame();
	}
}