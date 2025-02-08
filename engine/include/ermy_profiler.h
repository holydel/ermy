#pragma once
#include <ermy_api.h>

namespace ermy
{
	namespace profiler
	{
		void BeginScope(const char* name);
		void EndScope();
		void NextFrame();

		struct ProfileEvent
		{
			ProfileEvent(const char* name)
			{
				BeginScope(name);
			}
			~ProfileEvent()
			{
				EndScope();
			}				
		};
	}
}

#define ERMY_PROFILE_SCOPE(a) ermy::profiler::ProfileEvent profileEvent(name);

//todo __PRETTY_FUNCTION__  for gcc and clang
#define ERMY_PROFILE_FUNC() ERMY_PROFILE_SCOPE(__FUNCTION__)

