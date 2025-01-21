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

		//TODO: Think about tempalte variant with char8_t
		void EnqueueLogMessageRAWTagged(LogSeverity severity, const char* tag, const char* message, ...);
		void EnqueueLogMessageRAW(LogSeverity severity, const char* message, ...);
	}
}

#define ERMY_LOG(message,...) ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Debug,ermy::logger::Config().tag.c_str(), message, __VA_ARGS__)
#define ERMY_WARNING(message,...) ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Warning,ermy::logger::Config().tag.c_str(), message, __VA_ARGS__)
#define ERMY_INFO(message,...) ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Verbose,ermy::logger::Config().tag.c_str(), message, __VA_ARGS__)
#define ERMY_ERROR(message,...) ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Error,ermy::logger::Config().tag.c_str(), message, __VA_ARGS__)
#define ERMY_FATAL(message,...) ermy::logger::EnqueueLogMessageRAWTagged(ermy::LogSeverity::Fatal,ermy::logger::Config().tag.c_str(), message, __VA_ARGS__)