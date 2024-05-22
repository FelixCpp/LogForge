#pragma once

#include <ranges>
#include <unordered_map>

#include "../LogPrinter.hpp"

namespace LogForge
{

	typedef std::unordered_map<Severity, std::optional<Line>> SeverityPrefixes;

	inline static const SeverityPrefixes DefaultSeverityPrefixes = {
		{ Severity::Trace, L"[TRACE]: " },
		{ Severity::Debug, L"[DEBUG]: " },
		{ Severity::Info, L"[INFO]: " },
		{ Severity::Warning, L"[WARNING]: " },
		{ Severity::Error, L"[ERROR]: " },
		{ Severity::Fatal, L"[FATAL]: " }
	};

	template <std::derived_from<LogPrinter> Printer>
	class PrefixPrinter final : public LogPrinter
	{
	public:

		explicit PrefixPrinter(Printer realPrinter, SeverityPrefixes severityPrefixes = DefaultSeverityPrefixes) noexcept :
			RealPrinter(std::move(realPrinter)),
			SeverityPrefixes(std::move(severityPrefixes)),
			LongestPrefixLength(GetLongestPrefixLength(SeverityPrefixes | std::ranges::views::values))
		{}

		[[nodiscard]] Lines Print(const LogEvent& event) const override
		{
			auto printedLines = RealPrinter.Print(event);
			const auto prefix = GetPrefixForSeverity(event.Severity);
			if (not prefix.has_value()) return printedLines;

			const auto spacing = Line(LongestPrefixLength - prefix.value().length(), L' ');
			const auto prefixedLines = printedLines | std::ranges::views::transform([leading = prefix.value() + spacing](const Line& line)
			{
				return leading + line;
			});

			return { prefixedLines.begin(), prefixedLines.end() };
		}

	private:

		[[nodiscard]] std::optional<Line> GetPrefixForSeverity(const Severity severity) const
		{
			const auto prefix = SeverityPrefixes.find(severity);
			if (prefix != SeverityPrefixes.end())
			{
				return prefix->second;
			}

			return std::nullopt;
		}

		[[nodiscard]] static std::size_t GetLongestPrefixLength(const auto& prefixes)
		{
			return std::ranges::max(prefixes | std::ranges::views::transform([](const auto& prefix)
			{
				if (prefix.has_value())
				{
					return prefix->length();
				}

				return std::size_t(0);
			}));
		}

	public:

		Printer RealPrinter;
		SeverityPrefixes SeverityPrefixes;
		std::size_t LongestPrefixLength;

	};

	class PrefixPrinterBuilder
	{
	public:

		explicit PrefixPrinterBuilder(SeverityPrefixes severityPrefixes = DefaultSeverityPrefixes) noexcept :
			SeverityPrefixes(std::move(severityPrefixes))
		{}

		auto Build(const std::derived_from<LogPrinter> auto printer) const
		{
			return PrefixPrinter(std::move(printer), SeverityPrefixes);
		}

	public:

		SeverityPrefixes SeverityPrefixes;

	};

	inline auto Prefixed(SeverityPrefixes severityPrefixes = DefaultSeverityPrefixes)
	{
		return PrefixPrinterBuilder(std::move(severityPrefixes));
	}

}