#pragma once

#include <concepts>

namespace LogForge
{

	class LogPrinter;

	template <typename Printer, typename PrinterBuilder> requires requires(Printer printer, PrinterBuilder builder)
	{
		std::derived_from<Printer, LogPrinter>;
		{ builder.Build(printer) } -> std::derived_from<LogPrinter>;
	}
	constexpr auto operator >> (Printer printer, PrinterBuilder builder) noexcept
	{
		return builder.Build(printer);
	}

}
