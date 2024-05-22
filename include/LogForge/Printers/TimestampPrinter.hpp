#pragma once

#include "../LogPrinter.hpp"

#include <sstream>

namespace LogForge
{

	inline static constexpr auto DefaultTimeFormat = L"%d.%m.%Y %H:%M:%S";
	inline static constexpr auto DefaultTimePrefix = L"Time: ";

	template <std::derived_from<LogPrinter> Printer>
	class TimestampPrinter final : public LogPrinter
	{
	public:

		constexpr explicit TimestampPrinter(
			Printer realPrinter,
			Line timeFormat = DefaultTimeFormat,
			Line timePrefix = DefaultTimePrefix
		) noexcept :
			RealPrinter(std::move(realPrinter)),
			TimeFormat(std::move(timeFormat)),
			Prefix(std::move(timePrefix))
		{}

		[[nodiscard]] Lines Print(const LogEvent& event) const override
		{
			auto printedLines = RealPrinter.Print(event);
			const auto timeline = FormatTime(event.Time).value_or(L"<Invalid Time>");
			const auto prefixedTimeline = Prefix + timeline;

			Lines output;
			output.reserve(printedLines.size() + 1);
			output.insert(output.begin(), prefixedTimeline);
			output.insert(output.end(), printedLines.begin(), printedLines.end());
			return output;
		}

	private:

		[[nodiscard]] std::optional<Line> FormatTime(const TimePoint& timePoint) const
		{
			const auto time = Clock::to_time_t(timePoint);
			std::tm timeInfo;
			if (localtime_s(&timeInfo, &time) != 0) return std::nullopt;

			std::wostringstream wss;
			wss << std::put_time(&timeInfo, TimeFormat.c_str());
			return wss.str();
		}

	public:

		Printer RealPrinter;
		Line TimeFormat;
		Line Prefix;

	};

	class TimestampPrinterBuilder
	{
	public:

		constexpr explicit TimestampPrinterBuilder(
			Line timeFormat = DefaultTimeFormat,
			Line prefix = DefaultTimePrefix
		) noexcept :
			TimeFormat(std::move(timeFormat)),
			Prefix(std::move(prefix))
		{}

		auto Build(const std::derived_from<LogPrinter> auto printer) const
		{
			return TimestampPrinter(std::move(printer), TimeFormat, Prefix);
		}

	public:

		Line TimeFormat;
		Line Prefix;

	};

	constexpr auto Timestamped(
		Line timeFormat = DefaultTimeFormat,
		Line prefix = DefaultTimePrefix
	) noexcept -> decltype(TimestampPrinterBuilder(std::move(timeFormat), std::move(prefix)))
	{
		return TimestampPrinterBuilder(std::move(timeFormat), std::move(prefix));
	}

}