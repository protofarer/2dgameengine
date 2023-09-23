#include "Logger.h"
#include <iostream>
#include <chrono>
#include <ctime>

#define ANSI_COLOR_RED "\x1b[31m"
#define ANSI_COLOR_GREEN "\x1b[32m"
#define ANSI_COLOR_RESET "\x1b[0m"

// deine messages field
std::vector<LogEntry> Logger::messages;

std::string Logger::CurrentDateTimeToString() {
	std::time_t now = std::chrono::system_clock::to_time_t(
		std::chrono::system_clock::now());
	std::string output(30, '\0');
	std::strftime(&output[0], output.size(), "%Y/%m/%d %H:%M:%S", std::localtime(&now));
	return output;
}

void Logger::Log(const std::string& message) {
	LogEntry logEntry;
	logEntry.type = LOG_INFO;
	logEntry.message = "LOG | " + CurrentDateTimeToString() + " - " + message;

	std::cout << ANSI_COLOR_GREEN << logEntry.message << ANSI_COLOR_RESET << std::endl;

	messages.push_back(logEntry);
}

void Logger::Err(const std::string& message) {
	LogEntry logEntry;
	logEntry.type = LOG_ERROR;
	logEntry.message = "ERR | " + CurrentDateTimeToString() + " - " + message;

	std::cout << ANSI_COLOR_RED << logEntry.message << ANSI_COLOR_RESET << std::endl;

	messages.push_back(logEntry);
}