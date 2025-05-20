#pragma once

#include "ermy_api.h"
#include <string>
#include <cstdarg>

namespace ermy
{
	enum class LogSeverity
	{
		Verbose = 0,
		Debug = 1,
		Warning = 2,
		Error = 3,
		Fatal = 4
	};

	namespace logger
	{
		//Loggers: Console / IDE / Remote / OnScreen / File

		enum class LoggerType
		{
			Console,
			IDE,
			Remote,
			OnScreen,
			File
		};

		struct LoggersConfig
		{
			bool ConsoleMirroring = true;
			bool IDEMirroring = true;

			struct RemoteMirroringConfig
			{
				bool enabled = false;
				std::string remoteAddress = "127.0.0.1";
			} RemoteMirroring;

			struct FileMirroringConfig
			{
				bool enabled = false;
				std::u8string customFilePath = u8"";
			} FileMirroring;

			struct OnScreenMirroringConfig
			{
				bool enabled = true;
				float liveTime = 2.0f;
				int maxRecords = 50;
				int fontSize = 16;
			} OnScreenMirroring;

			std::u8string tag = u8"ErmyAPP";
		};

		LoggersConfig& Config();

		//TODO: Think about tempalte variant with char8_t
		void EnqueueLogMessageRAWTagged(LogSeverity severity, const char8_t* tag, const char8_t* message, ...);
		void EnqueueLogMessageRAW(LogSeverity severity, const char8_t* message, ...);
	}
}

#define ERMY_LOG(...) ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Debug,ermy::logger::Config().tag.c_str(), __VA_ARGS__)
#define ERMY_WARNING(...) ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Warning,ermy::logger::Config().tag.c_str(), __VA_ARGS__)
#define ERMY_INFO(...) ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Verbose,ermy::logger::Config().tag.c_str(), __VA_ARGS__)
#define ERMY_ERROR(...) ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Error,ermy::logger::Config().tag.c_str(), __VA_ARGS__)
#define ERMY_FATAL(...) ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Fatal,ermy::logger::Config().tag.c_str(), __VA_ARGS__)