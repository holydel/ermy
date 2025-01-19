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
				std::string customFilePath = "";
			} FileMirroring;

			struct OnScreenMirroringConfig
			{
				bool enabled = true;
				float liveTime = 2.0f;
				int maxRecords = 50;
				int fontSize = 16;
			} OnScreenMirroring;

			std::string tag = "ErmyAPP";
		};

		LoggersConfig& Config();

		void EnqueueLogMessageRAWTagged(LogSeverity severity, const char* tag, const char* message, va_list args);

		void EnqueueLogMessageRAW(LogSeverity severity, const char* message, ...);
	}
}