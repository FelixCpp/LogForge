#pragma once

#include <concepts>
#include <ranges>

#include "../LogPrinter.hpp"

namespace LogForge
{

	template <std::derived_from<LogPrinter> Printer>
	class BoxPrinter final : public LogPrinter
	{
	public:

		static constexpr auto TopLeft = L'┌';
		static constexpr auto TopRight = L'┐';
		static constexpr auto BottomLeft = L'└';
		static constexpr auto BottomRight = L'┘';
		static constexpr auto Horizontal = L'─';
		static constexpr auto Vertical = L'│';

		constexpr explicit BoxPrinter(Printer realPrinter) noexcept :
			RealPrinter(std::move(realPrinter))
		{
		}

		[[nodiscard]] Lines Print(const LogEvent& event) const override
		{
			auto printedLines = RealPrinter.Print(event);
			const auto longestLine = GetLongestLineLength(printedLines);
			if (not longestLine.has_value()) return printedLines;;

			const auto horizontalLine = Line(longestLine.value(), Horizontal);
			const auto upperLine = TopLeft + horizontalLine + TopRight;
			const auto lowerLine = BottomLeft + horizontalLine + BottomRight;

			const auto boxedLines = printedLines | std::ranges::views::transform([longestLine](const Line& line)
			{
				const auto spacingCount = longestLine.value() - line.length();
				const auto trailingSpacing = Line(spacingCount, L' ');

				return Vertical + line + trailingSpacing + Vertical;
			});

			Lines output;
			output.reserve(printedLines.size() + 2);
			output.insert(output.end(), upperLine);
			output.insert(output.end(), boxedLines.begin(), boxedLines.end());
			output.insert(output.end(), lowerLine);

			return output;
		}

	private:

		[[nodiscard]] static std::optional<std::size_t> GetLongestLineLength(const Lines& lines)
		{
			return std::ranges::max(lines | std::ranges::views::transform([](const Line& line) { return line.length(); }));
		}

	public:

		Printer RealPrinter;

	};

	class BoxPrinterBuilder final
	{
	public:

		constexpr BoxPrinterBuilder() noexcept = default;

		[[nodiscard]] constexpr auto Build(const std::derived_from<LogPrinter> auto printer) const noexcept -> decltype(BoxPrinter{ std::move(printer) })
		{
			return BoxPrinter{ std::move(printer) };
		}

	};

	constexpr auto Boxed() noexcept -> decltype(BoxPrinterBuilder {})
	{
		return BoxPrinterBuilder{};
	}

}
