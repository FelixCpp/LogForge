#pragma once

#include <string>
#include <vector>
#include <chrono>
#include <source_location>
#include <variant>

namespace LogForge
{

	/// Utility type definitions
	typedef std::wstring Line;
	typedef std::vector<Line> Lines;
	typedef std::chrono::system_clock Clock;
	typedef std::chrono::time_point<Clock> TimePoint;
	typedef std::source_location SourceLocation;

	typedef std::variant<Line, std::exception> LogMessage;

}