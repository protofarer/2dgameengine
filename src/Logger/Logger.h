#pragma once

#include <string>
#include <vector>

enum LogType {
	LOG_INFO,
	LOG_WARNING,
	LOG_ERROR
};
struct LogEntry {
	LogType type;
	std::string message;
};

class Logger {
	public:
		static std::vector<LogEntry> messages;
		static void Log(const std::string& message); // because passed object is a literal or const (stored in program data)
		static void Err(const std::string& message);
		static std::string CurrentDateTimeToString();
};