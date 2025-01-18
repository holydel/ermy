#include "logger.h"

#include <iostream>

ermy::logger::LoggersConfig gAppLoggerConfig = {};
ermy::logger::LoggersConfig gUsedLoggerConfig = {};

void loggerImpl::Initialize()
{
	gUsedLoggerConfig = gAppLoggerConfig; 
	//after app initialization we cannot change logger config
}

void loggerImpl::Shutdown()
{

}

void ermy::logger::EnqueueLogMessageRAW(ermy::LogSeverity severity, const char* tag, const char* message)
{
	std::cout << message << std::endl;
}

ermy::logger::LoggersConfig& ermy::logger::Config()
{
	return gAppLoggerConfig;
}