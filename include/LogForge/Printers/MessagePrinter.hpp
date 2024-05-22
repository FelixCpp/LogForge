#pragma once

#include <ranges>
#include <sstream>

#include "../LogPrinter.hpp"

namespace LogForge
{

	class MessagePrinter final : public LogPrinter
	{
	public:

		constexpr MessagePrinter() noexcept = default;

		[[nodiscard]] Lines Print(const LogEvent& event) const override
		{
			return std::visit([]<typename T>(const T& message) -> Lines
			{
				if constexpr (std::is_same_v<std::remove_cvref_t<T>, Line>)
				{
					auto lines = message | std::ranges::views::split(L'\n') | std::ranges::views::transform([](const auto& subrange)
					{
						return Line { subrange.begin(), subrange.end() };
					});

					return Lines { lines.begin(), lines.end() };
				}
				else if constexpr (std::is_same_v<std::remove_cvref_t<T>, std::exception>)
				{
					std::wostringstream wss;
					wss << L"Error: " << message.what();
					return { wss.str() };
				}
				else
				{
					return {};
				}
			}, event.Message);
		}

	};

	[[nodiscard]] constexpr auto Message() noexcept -> decltype(MessagePrinter {})
	{
		return MessagePrinter {};
	}
}