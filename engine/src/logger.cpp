#include "logger.h"
#include "os/os_utils.h"

#include <iostream>
#include <cstdarg>

ermy::logger::LoggersConfig gAppLoggerConfig = {};
ermy::logger::LoggersConfig gUsedLoggerConfig = {};

void loggerImpl::Initialize()
{
	gUsedLoggerConfig = gAppLoggerConfig;

	ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity::Debug, u8"logger initialized"); //
	// after app initialization we cannot change logger config
}

void loggerImpl::Shutdown()
{
}

void ermy::logger::EnqueueLogMessageRAW(LogSeverity severity, const char8_t* message, ...)
{
	va_list args;
	va_start(args, message);
	EnqueueLogMessageRAWTagged(severity, Config().tag.c_str(), message, args);
	va_end(args);
}

void ermy::logger::EnqueueLogMessageRAWTagged(LogSeverity severity, const char8_t*tag, const char8_t *message, ...)
{
	char8_t data[4096];

	va_list args;
	va_start(args, message);
	int lastSymbol = vsnprintf((char*)data, sizeof(data) - 2, (const char*)message, args);
	va_end(args);

	//[[assume(lastSymbol < 4096-2)]]

	data[lastSymbol] = '\n';
	data[lastSymbol+1] = '\0';

	if (gUsedLoggerConfig.ConsoleMirroring)
	{
		os::WriteDebugLogMessageConsole(severity, data);
	}
	if (gUsedLoggerConfig.IDEMirroring)
	{
		os::WriteDebugLogMessageIDE(severity, data);
	}
	if (gUsedLoggerConfig.FileMirroring.enabled)
	{		
		os::WriteDebugLogMessageFile(severity, data);
	}
	va_end(args);
}

ermy::logger::LoggersConfig &ermy::logger::Config()
{
	return gAppLoggerConfig;
}