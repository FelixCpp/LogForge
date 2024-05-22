#pragma once

#include <concepts>
#include <unordered_map>
#include <ranges>

#include "../LogPrinter.hpp"

namespace LogForge
{

	typedef std::unordered_map<Severity, std::optional<Line>> SeverityColors;

	inline static const SeverityColors DefaultSeverityColors = {
		{ Severity::Trace, L"\x1B[38;5;244m" },
		{ Severity::Debug, std::nullopt },
		{ Severity::Info, L"\x1B[38;5;12m" },
		{ Severity::Warning, L"\x1B[38;5;208m" },
		{ Severity::Error, L"\x1B[38;5;196m" },
		{ Severity::Fatal, L"\x1B[38;5;199m" }
	};

	template <std::derived_from<LogPrinter> Printer>
	class ColoredPrinter final : public LogPrinter
	{
	public:

		explicit ColoredPrinter(
			Printer realPrinter,
			SeverityColors severityColors = DefaultSeverityColors
		) noexcept :
			RealPrinter(std::move(realPrinter)),
			SeverityColors(std::move(severityColors))
		{}

		[[nodiscard]] Lines Print(const LogEvent& event) const override
		{
			auto printedLines = RealPrinter.Print(event);
			const auto color = GetColorForSeverity(event.Severity);
			if (not color.has_value()) return printedLines;

			const auto coloredLines = printedLines | std::ranges::views::transform([colorCode = color.value()](const Line& line)
			{
				return Colorize(line, colorCode);
			});

			return { coloredLines.begin(), coloredLines.end() };
		}

	private:

		[[nodiscard]] std::optional<Line> GetColorForSeverity(const Severity severity) const
		{
			const auto color = SeverityColors.find(severity);
			if (color != SeverityColors.end())
			{
				return color->second;
			}

			return std::nullopt;
		}

		[[nodiscard]] static Line Colorize(const Line& line, const Line& color)
		{
			static constexpr auto resetColor = L"\x1B[0m";
			return color + line + resetColor;
		}

	public:

		Printer RealPrinter;
		SeverityColors SeverityColors;

	};

	class ColoredPrinterBuilder
	{
	public:

		explicit ColoredPrinterBuilder(SeverityColors severityColors = DefaultSeverityColors) noexcept :
			SeverityColors(std::move(severityColors))
		{}

		[[nodiscard]] auto Build(const std::derived_from<LogPrinter> auto printer) const
		{
			return ColoredPrinter(std::move(printer), SeverityColors);
		}

	public:

		SeverityColors SeverityColors;

	};

	[[nodiscard]] inline auto Colored(const SeverityColors& severityColors = DefaultSeverityColors) noexcept -> ColoredPrinterBuilder
	{
		return ColoredPrinterBuilder(severityColors);
	}

}