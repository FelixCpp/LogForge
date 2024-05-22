#pragma once

#include "Types.hpp"
#include "Severity.hpp"

namespace LogForge
{

	/// Structure that represents a log event
	struct LogEvent
	{
		Severity Severity;				///< Severity of the log event
		LogMessage Message;				///< Message of the log event
		TimePoint Time;					///< Time of the log event
		SourceLocation SourceLocation;	///< Source location of the log event
	};

}