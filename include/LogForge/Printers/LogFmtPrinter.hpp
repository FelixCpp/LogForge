#pragma once

#include "../Severity.hpp"
#include "../LogPrinter.hpp"
#include "PrefixPrinter.hpp"

namespace LogForge
{

	class LogFmtPrinter final : public LogPrinter
	{
	public:

		inline static const SeverityPrefixes DefaultSeverityPrefixes = {
			{ Severity::Trace, L"trace" },
			{ Severity::Debug, L"debug" },
			{ Severity::Info, L"info" },
			{ Severity::Warning, L"warning" },
			{ Severity::Error, L"error" },
			{ Severity::Fatal, L"fatal" }
		};

		inline static const Line DefaultTimeFormat = L"%FT%T%z";

		explicit LogFmtPrinter(
			SeverityPrefixes severityPrefixes = DefaultSeverityPrefixes,
			Line timeFormat = DefaultTimeFormat
		) noexcept :
			SeverityPrefixes(std::move(severityPrefixes)),
			TimeFormat(std::move(timeFormat))
		{}

		[[nodiscard]] Lines Print(const LogEvent& event) const override
		{
			Lines lines;

			if (const auto prefix = GenerateLevel(event.Severity))
			{
				lines.emplace_back(prefix.value());
			}

			if (const auto message = GenerateMessage(event.Message))
			{
				lines.emplace_back(message.value());
			}

			if (const auto time = GenerateTime(event.Time))
			{
				lines.emplace_back(time.value());
			}

			Line line;
			for (std::size_t i = 0; i < lines.size(); ++i)
			{
				if (i > 0) line += L' ';
				line += lines[i];
			}

			return { line };
		}

	private:

		[[nodiscard]] std::optional<Line> GenerateLevel(const Severity severity) const
		{
			using namespace std::string_literals;
			const auto prefix = SeverityPrefixes.find(severity);
			if (prefix != SeverityPrefixes.end() and prefix->second.has_value())
			{
				return L"level="s + prefix->second.value();
			}

			return std::nullopt;
		}

		[[nodiscard]] static std::optional<Line> GenerateMessage(const LogMessage& message)
		{
			using namespace std::string_literals;
			return std::visit([]<typename T>(const T& msg) -> std::optional<Line>
			{
				if constexpr (std::is_same_v<std::decay_t<T>, Line>)
				{
					return L"message="s + msg;
				}
				else if constexpr (std::is_same_v<std::decay_t<T>, std::exception>)
				{
					std::wostringstream wss;
					wss << L"error=" << msg.what();
					return wss.str();
				}
				else
				{
					return {};
				}
			}, message);
		}

		[[nodiscard]] std::optional<Line> GenerateTime(const TimePoint& time) const
		{
			using namespace std::string_literals;
			const auto timePoint = Clock::to_time_t(time);
			tm timeInfo = {};
			if (localtime_s(&timeInfo, &timePoint) != 0)
			{
				return std::nullopt;
			}

			std::wostringstream wss;
			wss << L"time=" << std::put_time(&timeInfo, TimeFormat.data());
			return wss.str();
		}

	public:

		SeverityPrefixes SeverityPrefixes;
		Line TimeFormat;

	};

	[[nodiscard]] auto LogFmt(SeverityPrefixes severityPrefixes = LogFmtPrinter::DefaultSeverityPrefixes) noexcept -> decltype(LogFmtPrinter{ std::move(severityPrefixes) })
	{
		return LogFmtPrinter { std::move(severityPrefixes) };
	}

}