#pragma once

#include "LogPrinter.hpp"

namespace LogForge
{

	class Logger
	{
	public:

		virtual ~Logger() = default;
		virtual void Log(const LogEvent& event) const = 0;

		void Trace(const LogMessage& message, const TimePoint& time = Clock::now(), const SourceLocation& location = SourceLocation::current()) const
		{
			Log({ Severity::Trace, message, time, location });
		}

		void Debug(const LogMessage& message, const TimePoint& time = Clock::now(), const SourceLocation& location = SourceLocation::current()) const
		{
			Log({ Severity::Debug, message, time, location });
		}

		void Info(const LogMessage& message, const TimePoint& time = Clock::now(), const SourceLocation& location = SourceLocation::current()) const
		{
			Log({ Severity::Info, message, time, location });
		}

		void Warning(const LogMessage& message, const TimePoint& time = Clock::now(), const SourceLocation& location = SourceLocation::current()) const
		{
			Log({ Severity::Warning, message, time, location });
		}

		void Error(const LogMessage& message, const TimePoint& time = Clock::now(), const SourceLocation& location = SourceLocation::current()) const
		{
			Log({ Severity::Error, message, time, location });
		}

		void Fatal(const LogMessage& message, const TimePoint& time = Clock::now(), const SourceLocation& location = SourceLocation::current()) const
		{
			Log({ Severity::Fatal, message, time, location });
		}

	};

}