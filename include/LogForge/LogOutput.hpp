#pragma once

#include "LogPrinter.hpp"

namespace LogForge
{

	/// Structure that represents an output event
	struct OutputEvent
	{
		Lines		Lines;	///< Lines of the output event
		LogEvent	Origin;	///< Origin of the output event
	};

	class LogOutput
	{
	public:

		virtual ~LogOutput() = default;
		virtual void Output(const OutputEvent& event) const = 0;

	};

}