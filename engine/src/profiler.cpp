#include "profiler.h"
#include <vector>
#include <atomic>
#include <mutex>
#include <thread>
#include <stack>
#include <cassert>
#include "os/os_utils.h"
#include <string>
#include <unordered_map>
#include <array>
#include <functional>

using namespace ermy;

class ProfilerData
{
public:
	constexpr static int MAX_FRAMES_IN_WINDOW = 10'000;
private:
	struct ScopeInfo
	{
		const char* name;
		i64 timestamp;
	};

	struct FrameInfo
	{
		std::unordered_map<size_t, i64> scopeDurations;
	};

	std::unordered_map<size_t, std::string> humanReadableScopes;

	std::stack<ScopeInfo> scopes;
	std::array<FrameInfo, MAX_FRAMES_IN_WINDOW> frames;
	int currentFrame = 0;
public:
	ProfilerData();

	static ProfilerData* GetForCurrentThread();

	void BeginScope(const char* name);
	void EndScope();
	void NextFrame();
};

std::mutex gAllDataMutex;
std::vector<ProfilerData*> gAllThreadsData;
thread_local ProfilerData* gProfilerDataPerThread = nullptr;
thread_local std::hash<std::string> gStringHash;

void profiler_impl::Initialize()
{

}

void profiler_impl::Shutdown()
{

}

void ermy::profiler::BeginScope(const char* name)
{
	
}

void ermy::profiler::EndScope()
{
}

void ermy::profiler::NextFrame()
{
	for (auto tdata : gAllThreadsData)
	{
		tdata->NextFrame();
	}
}

ProfilerData::ProfilerData()
{
	std::lock_guard<std::mutex> lock(gAllDataMutex);

	gAllThreadsData.push_back(this);
}

ProfilerData* ProfilerData::GetForCurrentThread()
{
	if (!gProfilerDataPerThread)
	{
		gProfilerDataPerThread = new ProfilerData();
	}

	return gProfilerDataPerThread;
}

void ProfilerData::BeginScope(const char* name)
{
	scopes.push({ name, os::GetCurrentTimestamp()});
}

void ProfilerData::EndScope()
{
	assert(!scopes.empty());

	
	u64 durationClock = os::GetCurrentTimestamp() - scopes.top().timestamp;

	std::string scopeName = scopes.top().name;
	auto shash = gStringHash(scopeName);

	if (humanReadableScopes.find(shash) == humanReadableScopes.end())
	{
		humanReadableScopes[shash] = scopeName;
	}

	frames[currentFrame].scopeDurations[shash] = durationClock;
	scopes.pop();
}

void ProfilerData::NextFrame()
{
	currentFrame++;
}