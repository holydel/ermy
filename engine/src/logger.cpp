#include "logger.h"
#include "os/os_utils.h"

#include <iostream>
#include <cstdarg>

ermy::logger::LoggersConfig gAppLoggerConfig = {};
ermy::logger::LoggersConfig gUsedLoggerConfig = {};

void loggerImpl::Initialize()
{
	gUsedLoggerConfig = gAppLoggerConfig;

	ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Debug, "logger initialized"); //
	// after app initialization we cannot change logger config
}

void loggerImpl::Shutdown()
{
}

void ermy::logger::EnqueueLogMessageRAW(LogSeverity severity, const char* message, ...)
{
	va_list args;
	va_start(args, message);
	EnqueueLogMessageRAWTagged(severity, Config().tag.c_str(), message, args);
	va_end(args);
}

void ermy::logger::EnqueueLogMessageRAWTagged(LogSeverity severity, const char *tag, const char *message, ...)
{
	// Determine the required buffer size
	//auto v = va_arg(args, const char*);
	char data[4096];
	//int size = vsprintf_s(data, message, args);
	//va_list args_copy;
	//va_copy(args_copy, args);
	//int size = std::snprintf(nullptr, 0, message, args_copy);
	//va_end(args_copy);

	//char *data = static_cast<char *>(alloca(size + 2));

	// Format the string
	va_list args;
	va_start(args, message);
	int size = std::vsnprintf(data, sizeof(data)-2, message, args);
	data[size] = '\n';
	data[size + 1] = '\0';

	if (gUsedLoggerConfig.ConsoleMirroring)
	{
		std::cout << data;
	}
	if (gUsedLoggerConfig.IDEMirroring)
	{
		os::WriteDebugLogMessageIDE(data);
	}
	va_end(args);
}

ermy::logger::LoggersConfig &ermy::logger::Config()
{
	return gAppLoggerConfig;
}