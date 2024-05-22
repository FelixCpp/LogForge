#pragma once

#include "../LogPrinter.hpp"
#include "../LogOutput.hpp"
#include "../LogFilter.hpp"
#include "../Logger.hpp"

namespace LogForge
{

	template <std::derived_from<LogFilter> Filter, std::derived_from<LogOutput> Output, std::derived_from<LogPrinter> Printer>
	class DefaultLogger final : public Logger
	{
	public:

		constexpr explicit DefaultLogger(Filter filter, Output output, Printer printer) noexcept :
			LogFilter(std::move(filter)),
			LogOutput(std::move(output)),
			LogPrinter(std::move(printer))
		{
		}

		void Log(const LogEvent& event) const override
		{
			if (LogFilter.Filter(event))
			{
				const auto outputEvent = OutputEvent {
					.Lines = LogPrinter.Print(event),
					.Origin = event
				};

				LogOutput.Output(outputEvent);
			}
		}

	public:

		Filter LogFilter;
		Output LogOutput;
		Printer LogPrinter;

	};

}