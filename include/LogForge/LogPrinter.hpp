#pragma once

#include "LogEvent.hpp"

namespace LogForge
{
	
	class LogPrinter
	{
	public:

		virtual ~LogPrinter() = default;
		[[nodiscard]] virtual Lines Print(const LogEvent& event) const = 0;

	};
}