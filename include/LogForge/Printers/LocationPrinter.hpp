#pragma once

#include <functional>
#include <sstream>

#include "../LogPrinter.hpp"

namespace LogForge
{

	typedef std::function<Line(SourceLocation)> SourceLocationFormatter;

	inline static constexpr auto DefaultLocationPrefix = L"Location: ";

	inline Line DefaultSourceLocationFormatter(const SourceLocation& location)
	{
		std::wstringstream stream;
		stream << location.file_name() << L"(" << location.line() << L", " << location.column() << "): " << location.function_name();
		return stream.str();
	}

	template <std::derived_from<LogPrinter> Printer>
	class LocationPrinter final : public LogPrinter
	{
	public:

		explicit LocationPrinter(
			Printer realPrinter,
			SourceLocationFormatter sourceLocationFormatter = &DefaultSourceLocationFormatter,
			Line prefix = DefaultLocationPrefix
		) noexcept :
			RealPrinter(std::move(realPrinter)),
			SourceLocationFormatter(std::move(sourceLocationFormatter)),
			Prefix(std::move(prefix))
		{}

		[[nodiscard]] Lines Print(const LogEvent& event) const override
		{
			auto printedLines = RealPrinter.Print(event);
			const auto locationLine = FormatLocation(event.SourceLocation).value_or(L"<Invalid Location>");
			const auto prefixedLocationLine = Prefix + locationLine;

			Lines output;
			output.reserve(printedLines.size() + 1);
			output.insert(output.begin(), prefixedLocationLine);
			output.insert(output.end(), printedLines.begin(), printedLines.end());
			return output;
		}

	private:

		[[nodiscard]] std::optional<Line> FormatLocation(const SourceLocation& location) const
		{
			if (SourceLocationFormatter == nullptr) return std::nullopt;
			return SourceLocationFormatter(location);
		}

	public:

		Printer RealPrinter;
		SourceLocationFormatter SourceLocationFormatter;
		Line Prefix;

	};

	class LocationPrinterBuilder
	{
	public:

		explicit LocationPrinterBuilder(
			SourceLocationFormatter sourceLocationFormatter = &DefaultSourceLocationFormatter,
			Line prefix = DefaultLocationPrefix
		) noexcept :
			SourceLocationFormatter(std::move(sourceLocationFormatter)),
			Prefix(std::move(prefix))
		{}

		[[nodiscard]] auto Build(const std::derived_from<LogPrinter> auto printer) const
		{
			return LocationPrinter(std::move(printer), SourceLocationFormatter, Prefix);
		}

	public:

		SourceLocationFormatter SourceLocationFormatter;
		Line Prefix;

	};

	[[nodiscard]] inline auto Located(
		SourceLocationFormatter sourceLocationFormatter = &DefaultSourceLocationFormatter,
		Line prefix = DefaultLocationPrefix
	) noexcept -> decltype(LocationPrinterBuilder(std::move(sourceLocationFormatter), std::move(prefix)))
	{
		return LocationPrinterBuilder(std::move(sourceLocationFormatter), std::move(prefix));
	}

}